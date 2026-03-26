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

    explicit StatusIndicator(QWidget *parent = nullptr);
    ~StatusIndicator();

    void setIcon(const QPixmap &icon);
    void setLabelText(const QString &text);
    void setValueText(const QString &text);

    State state() const { return m_state; }
    void setState(State state);
    bool isActive() const { return m_state == State::Active; }

private:
    void updateStyle();

    Ui::StatusIndicator *ui;
    State m_state;
};

#endif  // STATUSINDICATOR_H
