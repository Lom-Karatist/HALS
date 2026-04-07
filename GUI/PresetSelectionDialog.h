#ifndef PRESETSELECTIONDIALOG_H
#define PRESETSELECTIONDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>

class PresetSelectionDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PresetSelectionDialog(const QStringList &presets, QWidget *parent = nullptr);
    QString selectedPreset() const;

private:
    QComboBox *m_combo;
    QPushButton *m_okButton;
    QPushButton *m_cancelButton;
};

#endif // PRESETSELECTIONDIALOG_H