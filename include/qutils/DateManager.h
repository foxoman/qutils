#pragma once
#include <QObject>
#include <QDate>

namespace zmc
{

class DateManager : public QObject
{
    Q_OBJECT
public:
    explicit DateManager(QObject *parent = 0);

public:
    /**
     * @brief Adds the given months to the current date
     * @param months
     */
    Q_INVOKABLE void addMonths(const unsigned int &months);

    /**
     * @brief Add months to the given date and returns the new instance
     * @param from
     * @param months
     * @return
     */
    Q_INVOKABLE QDate addMonths(QDate from, const unsigned int &months);

    /**
     * @brief Add years to the given date and returns the new instance
     * @param from
     * @param years
     * @return
     */
    Q_INVOKABLE QDate addYears(QDate from, const unsigned int &years);

    /**
     * @brief Sets the day but keeps the month and the year the same
     * @param day
     */
    Q_INVOKABLE void setDay(const unsigned int &day);

    /**
     * @brief Only sets the month and resets the day to 1
     * @param month
     */
    Q_INVOKABLE void setMonth(const unsigned int &month);

    /**
     * @brief Sets the year but keeps the month and the day the same.
     * @param year
     */
    Q_INVOKABLE void setYear(const unsigned int &year);

    Q_INVOKABLE int getYear() const;
    Q_INVOKABLE int getDay() const;
    Q_INVOKABLE int getMonth() const;

    Q_INVOKABLE void setDate(const unsigned int &year, const unsigned int &month, const unsigned int &day);
    Q_INVOKABLE void setDate(const QDate &date);

    Q_INVOKABLE int getMonthLength(unsigned int year, unsigned int month) const;
    Q_INVOKABLE int getCurrentMonthLength() const;

    Q_INVOKABLE int getMonthStartIndex() const;
    Q_INVOKABLE QString getMonthName(const unsigned int &month) const;
    Q_INVOKABLE QString getCurrentMonthName() const;

    /**
     * @brief Creates a new instance from given parameters.
     * @param year
     * @param month
     * @param day
     * @return
     */
    Q_INVOKABLE QDate getDate(int year, int month, int day) const;

    /**
     * @brief Returns the date this instance is set to
     * @return
     */
    Q_INVOKABLE QDate getDate() const;

    /**
     * @brief Returns the current date
     * @return
     */
    Q_INVOKABLE QDate getCurrentDate() const;

    Q_INVOKABLE QString getDateString(const QString &format);

private:
    QDate m_Date;
};

}