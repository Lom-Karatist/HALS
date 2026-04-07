#include "PresetSelectionDialog.h"

PresetSelectionDialog::PresetSelectionDialog(const QStringList &presets, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Выбор пресета настроек");
    setModal(true);

    m_combo = new QComboBox(this);
    m_combo->addItems(presets);
    m_okButton = new QPushButton("OK", this);
    m_cancelButton = new QPushButton("Отмена", this);

    connect(m_okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_combo);
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(m_okButton);
    buttonLayout->addWidget(m_cancelButton);
    layout->addLayout(buttonLayout);
}

QString PresetSelectionDialog::selectedPreset() const
{
    return m_combo->currentText();
}