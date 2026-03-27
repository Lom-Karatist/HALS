#ifndef STATUSINDICATOR_H
#define STATUSINDICATOR_H

#include <QWidget>

namespace Ui {
class StatusIndicator;
}

class StatusIndicator : public QWidget {
    Q_OBJECT

public:
    enum class State {
        Active,    // зелёный
        Inactive,  // красный
        Unknown    // серый/белый
    };
    Q_ENUM(State)

    explicit StatusIndicator(QWidget *parent = nullptr, QString baseName = "");
    ~StatusIndicator();

    void setIconBaseName(const QString &text);
    void setIcon(QString nameAppendix);
    void setLabelText(const QString &text);
    void setValueText(const QString &text);

    State state() const { return m_state; }
    void setState(State state);
    bool isActive() const { return m_state == State::Active; }

private:
    void updateStyle();

    Ui::StatusIndicator *ui;
    State m_state;
    QString m_iconBaseName;
};

#endif  // STATUSINDICATOR_H
