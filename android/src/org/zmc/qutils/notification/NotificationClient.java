package org.zmc.qutils.notification;

import org.qtproject.qt5.android.bindings.QtActivity;
import android.app.Notification;
import android.app.NotificationManager;
import android.content.Context;

import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.Intent;

import android.graphics.Color;
import android.os.SystemClock;

import android.net.Uri;
import android.os.Build;

// qutils
import org.zmc.qutils.CppCallbacks;

public class NotificationClient extends QtActivity
{
    private static NotificationManager m_NotificationManager;

    // This is the main launcher context of the app
    private static QtActivity m_MainContext;

    private static int m_Priority = Notification.PRIORITY_DEFAULT;
    private static String m_Category = "";

    private static String m_LedColor = "";
    private static int m_LedOnMS = -1;
    private static int m_LedOffMS = -1;

    private static String m_Sound = "";

    private static int m_Defaults = -1;
    private static int m_Flags = -1;
    private static int m_Visibility = -2;

    private static String m_NotificationTag = "";
    private static int m_NotificationID = 0;
    // This is used to figure out which NotificationManager instance to send the signal in C++ side
    private static String m_TargetNotificationManager = "";

    private static int m_SmallIcon = -1;

    public static void resetProperties() {
        m_Priority = Notification.PRIORITY_DEFAULT;
        m_Category = "";
        m_LedColor = "";

        m_LedOnMS = -1;
        m_LedOffMS = -1;
        m_Sound = "";

        m_Defaults = -1;
        m_Flags = -1;
        m_Visibility = -2;

        m_NotificationTag = "";
        m_TargetNotificationManager = "";
    }

    public NotificationClient(QtActivity mainActivity)
    {
        m_MainContext = mainActivity;
    }

    public static boolean isInitialized()
    {
        return m_MainContext != null;
    }

    public static int getLastNotificationID() {
        return m_NotificationID;
    }

    public static void setPriority(int priority)
    {
        m_Priority = priority;
    }

    public static void setCategory(String category)
    {
        m_Category = category;
    }

    public static void setLedColor(String color)
    {
        m_LedColor = color;
    }

    public static void setLedOnMS(int ms)
    {
        m_LedOnMS = ms;
    }

    public static void setLedOffMS(int ms)
    {
        m_LedOffMS = ms;
    }

    public static void setSound(String sound)
    {
        m_Sound = sound;
    }

    public static void setDefaults(int defaults)
    {
        m_Defaults = defaults;
    }

    public static void setFlags(int flags)
    {
        m_Flags = flags;
    }

    public static void setVisibility(int visibility)
    {
        m_Visibility = visibility;
    }

    public static void setNotificationTag(String tag)
    {
        m_NotificationTag = tag;
    }

    public static void setTargetNotificationManager(String target)
    {
        m_TargetNotificationManager = target;
    }

    public static void setSmallIcon(int icon)
    {
        m_SmallIcon = icon;
    }

    public static void notify(String text, String title, String payload, long delayMilliseconds)
    {
        Notification notification = getNotification(text, title, payload);
        if (delayMilliseconds > 0) {
            scheduleNotification(notification, payload, delayMilliseconds);
        }
        else {
            if (m_NotificationManager == null) {
                m_NotificationManager = (NotificationManager)m_MainContext.getSystemService(Context.NOTIFICATION_SERVICE);
            }

            CppCallbacks.notificationReceived(m_NotificationTag, m_NotificationID, m_TargetNotificationManager, payload);
            m_NotificationManager.notify(m_NotificationID, notification);
        }

        m_NotificationID++;
    }

    private static void scheduleNotification(Notification notification, String payload, long delay)
    {
        Intent notificationIntent = new Intent(m_MainContext, NotificationReceiver.class);
        notificationIntent.putExtra(NotificationReceiver.KEY_NOTIFICATION_ID, m_NotificationID);
        if (m_NotificationTag.length() > 0) {
            notificationIntent.putExtra(NotificationReceiver.KEY_NOTIFICATION_TAG, m_NotificationTag);
        }
        notificationIntent.putExtra(NotificationReceiver.KEY_NOTIFICATION, notification);
        if (payload.length() > 0) {
            notificationIntent.putExtra(NotificationReceiver.KEY_PAYLOAD, payload);
        }
        PendingIntent pendingIntent = PendingIntent.getBroadcast(m_MainContext, 0, notificationIntent, PendingIntent.FLAG_UPDATE_CURRENT);

        long futureInMillis = SystemClock.elapsedRealtime() + delay;
        AlarmManager alarmManager = (AlarmManager)m_MainContext.getSystemService(Context.ALARM_SERVICE);
        alarmManager.set(AlarmManager.ELAPSED_REALTIME_WAKEUP, futureInMillis, pendingIntent);
    }

    private static Notification getNotification(String content, String title, String payload)
    {
        Notification.Builder builder = new Notification.Builder(m_MainContext);
        builder.setContentTitle(title);
        builder.setContentText(content);
        builder.setSmallIcon(m_SmallIcon);

        String packageName = m_MainContext.getPackageName();
        Intent contentIntent = m_MainContext.getPackageManager().getLaunchIntentForPackage(packageName);
        contentIntent.putExtra(NotificationReceiver.KEY_NOTIFICATION_ID, m_NotificationID);
        if (m_NotificationTag.length() > 0) {
            contentIntent.putExtra(NotificationReceiver.KEY_NOTIFICATION_TAG, m_NotificationTag);
        }
        if (m_TargetNotificationManager.length() > 0) {
            contentIntent.putExtra(NotificationReceiver.KEY_NOTIFICATION_MANAGER, m_TargetNotificationManager);
        }
        contentIntent.putExtra(NotificationReceiver.KEY_PAYLOAD, payload);

        PendingIntent pendingContentIntent = PendingIntent.getActivity(m_MainContext, 0, contentIntent, PendingIntent.FLAG_UPDATE_CURRENT);
        builder.setContentIntent(pendingContentIntent);

        if (m_Defaults != -1) {
            builder.setDefaults(m_Defaults);
        }

        builder.setPriority(m_Priority);

        if (m_LedColor.length() > 0) {
            builder.setLights(Color.parseColor(m_LedColor), m_LedOnMS, m_LedOffMS);
        }

        if (m_Sound.length() > 0) {
            builder.setSound(Uri.parse(m_Sound));
        }

        if (Build.VERSION.SDK_INT >= 21) {
            if (m_Visibility != -2) {
                builder.setVisibility(m_Visibility);
            }

            if (m_Category.length() > 0) {
                builder.setCategory(m_Category);
            }
        }

        Notification notification = builder.build();
        if (m_Flags != -1) {
            notification.flags = m_Flags;
        }

        return notification;
    }
}
