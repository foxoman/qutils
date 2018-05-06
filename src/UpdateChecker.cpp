#include "qutils/UpdateChecker.h"
// Qt
#include <QXmlStreamReader>
#include <QCoreApplication>
#include <QFileInfo>
#include <QDir>
// Local
#include "qutils/Macros.h"

namespace zmc
{

UpdateInfo::UpdateInfo(QObject *parent)
    : QObject(parent)
    , m_Name("")
    , m_Version("")
    , m_UpdateSize(0)
{

}

QString UpdateInfo::name() const
{
    return m_Name;
}

void UpdateInfo::setName(QString name)
{
    m_Name = name;
}

QString UpdateInfo::version() const
{
    return m_Version;
}

void UpdateInfo::setVersion(QString ver)
{
    m_Version = ver;
}

quint64 UpdateInfo::updateSize() const
{
    return m_UpdateSize;
}

void UpdateInfo::setUpdateSize(quint64 size)
{
    m_UpdateSize = size;
}

bool UpdateInfo::valid() const
{
    bool isValid = false;
    isValid |= m_Name.length() > 0;
    isValid |= m_Version.length() > 0;
    isValid |= m_UpdateSize > 0;

    return isValid;
}

// ----- Start UpdateChecker Implementation ----- //

UpdateChecker::UpdateChecker(QObject *parent)
    : QObject(parent)
    , m_MaintenanceToolName("maintenancetool")
    , m_Process(this)
{
    connect(&m_Process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &UpdateChecker::onProcessFinished);
    connect(&m_Process, &QProcess::errorOccurred, this, &UpdateChecker::onProcessErrorOcurred);
}

void UpdateChecker::checkForUpdates()
{
    const QString toolPath = getMaintenanceToolPath();
    m_Process.setArguments(QStringList {"--checkupdates"});
    m_Process.setProgram(toolPath);
    m_Process.start(QIODevice::ReadOnly);
}

QString UpdateChecker::maintenanceToolName() const
{
    return m_MaintenanceToolName;
}

void UpdateChecker::setMaintenanceToolName(const QString &name)
{
    const bool isChanged = name != m_MaintenanceToolName;
    if (isChanged) {
        m_MaintenanceToolName = name;
        emit maintenanceToolNameChanged();
    }
}

QString UpdateChecker::getMaintenanceToolPath() const
{
    QString relativePath = "";
#if defined(Q_OS_MACOS)
    relativePath = "../../../" + m_MaintenanceToolName + ".app/Contents/MacOS/" + m_MaintenanceToolName;
#elif defined(Q_OS_WIN)
    relativePath = "./" + m_MaintenanceToolName + ".exe";
#endif // OS Check

    QFileInfo fileInfo(QDir(qApp->applicationDirPath()), relativePath);
    if (!fileInfo.exists()) {
        LOG_ERROR("Cannot find the maintenance tool in the path: " << fileInfo.absoluteFilePath());
    }

    return fileInfo.absoluteFilePath();
}

void UpdateChecker::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode);
    if (exitStatus == QProcess::NormalExit) {
        const QByteArray output = m_Process.readAll();
        const QList<zmc::UpdateInfo *> updates = parseOutPut(output);
        QList<QObject *> ubjectList;
        for (UpdateInfo *i : updates) {
            ubjectList.append(static_cast<QObject *>(i));
        }

        if (updates.size() > 0) {
            updateAvailable(ubjectList);

            for (UpdateInfo *info : updates) {
                info->deleteLater();
            }
        }
    }
}

void UpdateChecker::onProcessErrorOcurred(QProcess::ProcessError error)
{
    emit errorOcurred(error);
}

QList<zmc::UpdateInfo *> UpdateChecker::parseOutPut(const QByteArray &output)
{
    QList<UpdateInfo *> updates;
    const QString outString = QString::fromUtf8(output);
    const int xmlBegin = outString.indexOf(QStringLiteral("<updates>"));
    if (xmlBegin < 0) {
        return updates;
    }

    const int xmlEnd = outString.indexOf(QStringLiteral("</updates>"), xmlBegin);
    if (xmlEnd < 0) {
        return updates;
    }

    QXmlStreamReader reader(outString.mid(xmlBegin, (xmlEnd + 10) - xmlBegin));
    reader.readNextStartElement();
    // Should always work because it was search for
    Q_ASSERT(reader.name() == QStringLiteral("updates"));

    while (reader.readNextStartElement()) {
        if (reader.name() != QStringLiteral("update")) {
            continue;
        }

        UpdateInfo *info = new UpdateInfo(this);
        info->setName(reader.attributes().value(QStringLiteral("name")).toString());
        info->setVersion(reader.attributes().value(QStringLiteral("version")).toString());
        info->setUpdateSize(reader.attributes().value(QStringLiteral("size")).toULongLong());

        if (info->valid()) {
            updates.append(info);
        }
        else {
            info->deleteLater();
        }
    }

    if (reader.hasError()) {
        LOG_ERROR("XML-reader-error: " << reader.errorString());
    }

    return updates;
}

}