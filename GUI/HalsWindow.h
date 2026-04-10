#ifndef HALSWINDOW_H
#define HALSWINDOW_H

#include <QMainWindow>

#include "Components/HalsFacade.h"
#include "DeviceParametersForm.h"
#include "OverlayLabel.h"
#include "PresetSelectionDialog.h"
#include "StatusIndicator.h"
#include "VirtualKeyboard.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class HalsWindow;
}
QT_END_NAMESPACE

/**
 * @brief Главное окно приложения ГАЛС.
 *
 * Класс управляет графическим интерфейсом пользователя на базе QMainWindow.
 * Обеспечивает:
 * - отображение главной страницы, страницы настроек и страницы камеры
 * (QStackedWidget);
 * - обработку сенсорного ввода (свайп для переключения страниц);
 * - отображение статуса устройств (GPS, USB, камеры, датчик освещённости,
 * полётное задание);
 * - передачу пользовательских параметров в HalsFacade;
 * - отображение видео с камер и тепловой карты гиперспектральных данных;
 * - вызов виртуальной клавиатуры для сенсорного ввода.
 *
 * @see HalsFacade
 * @see DeviceParametersForm
 * @see StatusIndicator
 */
class HalsWindow : public QMainWindow {
    Q_OBJECT

public:
    /**
     * @brief Конструктор главного окна.
     * @param parent Родительский виджет (по умолчанию nullptr).
     *
     * Выполняет начальную настройку проекта (загрузка настроек, инициализация
     * полей), настройку GUI и создание основных объектов (HalsFacade, таймеры).
     */
    HalsWindow(QWidget *parent = nullptr);

    /**
     * @brief Деструктор.
     *
     * Останавливает таймер обновления времени, удаляет фасад и очищает UI.
     */
    ~HalsWindow();

    /**
     * @brief Фильтр событий для обработки сенсорных свайпов на QStackedWidget.
     * @param watched Объект, на котором произошло событие.
     * @param event Событие.
     * @return true, если событие обработано, иначе false.
     *
     * Обрабатывает TouchBegin и TouchEnd на stackedWidget.
     * При свайпе вправо более чем на 100 пикселей переключает страницы:
     * - со страницы настроек на главную,
     * - с главной на страницу настроек.
     */
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    /**
     * @brief Завершает работу приложения после подтверждения пользователя.
     */
    void on_pushButtonQuit_clicked();

    /**
     * @brief Обновляет состояние USB и перепроверяет наличие полётного задания.
     */
    void on_pushButtonUpdateConfiguration_clicked();

    /**
     * @brief Запускает или останавливает эксперимент.
     *
     * При запуске меняет текст и иконку кнопки, вызывает
     * HalsFacade::startExperiment(). При остановке –
     * HalsFacade::stopExperiment().
     */
    void on_pushButtonStartStop_clicked();

    /**
     * @brief Устанавливает количество спутников GPS на индикаторе.
     * @param satellitesCount Количество видимых спутников.
     */
    void setSatellitesCount(const int &satellitesCount);

    /**
     * @brief Обновляет отображение температуры процессора.
     * @param temperature Температура в градусах Цельсия (строка с числом).
     */
    void updateCpuTemperature(QString temperature);

    /**
     * @brief Обновляет состояние USB-накопителя.
     * @param mounted true – накопитель подключён и смонтирован.
     * @param availableBytes Доступное свободное место в байтах.
     * @param totalBytes Общий объём накопителя в байтах.
     */
    void updateUsbState(bool mounted, qint64 availableBytes, qint64 totalBytes);

    /**
     * @brief Обновляет индикатор загрузки полётного задания.
     * @param loaded true – задание успешно загружено с USB.
     */
    void updateMissionLoaderState(bool loaded);

    /**
     * @brief Обновляет индикатор состояния гиперспектральной камеры.
     * @param connectionStatus true – камера подключена и инициализирована.
     */
    void updateHsState(bool connectionStatus);

    /**
     * @brief Обновляет индикатор состояния обзорной камеры.
     * @param connectionStatus true – камера подключена и инициализирована.
     */
    void updateOcState(bool connectionStatus);

    /**
     * @brief Отображает изображение с обзорной камеры и обновляет
     * характеристики.
     * @param imageToShow Кадр для отображения.
     * @param maxBrightness Максимальная яркость кадра (используется для шкалы).
     */
    void updateOcImageLabel(QImage imageToShow, int maxBrightness);

    /**
     * @brief Отображает тепловую карту гиперспектральных данных.
     * @param imageToShow Изображение тепловой карты.
     * @param maxBrightness Максимальное значение в поддиапазонах.
     */
    void updateHsImageLabel(QImage imageToShow, int maxBrightness);

    /**
     * @brief Обновляет рассчитанные характеристики для обзорной камеры.
     * @param fovMeters Поле зрения в метрах на заданной высоте.
     * @param gsd Размер пикселя на местности (Ground Sampling Distance) в
     * метрах.
     */
    void updateOcChars(double fovMeters, double gsd);

    /**
     * @brief Обновляет рассчитанные характеристики для гиперспектрального
     * сенсора.
     * @param fovMeters Поле зрения в метрах.
     * @param gsd Размер пикселя на местности (GSD) в метрах.
     */
    void updateHsChars(double fovMeters, double gsd);

    /**
     * @brief Форматирует количество байт в гигабайты с одним десятичным знаком.
     * @param bytes Количество байт.
     * @return Строка вида "X.X".
     */
    QString formatBytes(qint64 bytes);

    /**
     * @brief Обновляет отображение текущего времени (UTC+3) на верхней панели.
     *
     * Вызывается каждую секунду по таймеру.
     */
    void updateTime();

    /**
     * @brief Делает одиночный снимок пары кадров (гиперспектральный +
     * обзорный).
     */
    void on_pushButtonMakeSnapshot_clicked();

    /**
     * @brief Устанавливает прямоугольник поля зрения спектрометра на обзорном
     * изображении.
     * @param rect Прямоугольник в координатах оригинального изображения.
     *
     * Сохраняет значения в QSettings и обновляет отображение на OverlayLabel.
     */
    void setSpectrometerFovRect(const QRect &rect);

    /**
     * @brief Открывает диалог выбора пресета настроек.
     *
     * Сканирует доступные пресеты и при подтверждении вызывает
     * HalsFacade::loadPreset().
     */
    void on_pushButtonChoosePreset_clicked();

    /**
     * @brief Принудительно обновляет значение параметра в GUI (синхронизация с
     * железом).
     * @param type Тип параметра.
     * @param value Новое значение.
     *
     * @see HalsFacade::parameterValueChanged
     */
    void onForceParameterChanging(ParameterType type, int value);

    /**
     * @brief Обновляет индикатор состояния датчика освещённости.
     * @param connected true – датчик активен, false – не подключён.
     */
    void updateLightSensorState(bool connected);

    /**
     * @brief Показывает виртуальную клавиатуру для ввода числа в QSpinBox.
     * @param spinBox Целевой spin box.
     * @param rightAligned Флаг выравнивания клавиатуры (true – правая колонка).
     */
    void showKeyboard(QSpinBox *spinBox, bool rightAligned);

private:
    /**
     * @brief Сканирует директорию приложения на наличие файлов пресетов.
     * @return Список имён пресетов (первым всегда "По умолчанию").
     *
     * Ищет файлы вида HALS_*.ini, HS_*.ini, LS_*.ini, OC_*.ini с одинаковым
     * именем.
     */
    QStringList scanAvailablePresets();

    /**
     * @brief Общая настройка проекта: загрузка настроек, инициализация UI.
     */
    void setupProject();

    /**
     * @brief Инициализация фасада, таймеров и подключение сигналов.
     */
    void initObjects();

    /**
     * @brief Настройка графического интерфейса: иконки, стили, фильтры событий.
     */
    void setupGui();

    /**
     * @brief Инициализация форм настроек (сенсор ГС, обзорная камера,
     * освещённость, эксперимент).
     */
    void initSettingsForms();

    /**
     * @brief Настраивает одну форму с двумя параметрами.
     * @param form Указатель на форму.
     * @param deviceName Имя устройства (отображается в заголовке формы).
     * @param firstParameterInfo Информация о первом параметре.
     * @param secondParameterInfo Информация о втором параметре.
     */
    void setupSettingBox(DeviceParametersForm *form, QString deviceName,
                         ParameterInfo firstParameterInfo,
                         ParameterInfo secondParameterInfo);

    /**
     * @brief Подключает сигнал requestKeyboard от всех ParameterModificator
     * внутри формы.
     * @param form Форма настроек.
     */
    void connectKeyboardForForm(DeviceParametersForm *form);

    /**
     * @brief Применяет QSS-стили из ресурсов.
     */
    void applyStyleSheet();

    /**
     * @brief Добавляет индикаторы статуса на главную страницу.
     */
    void addStatusIndicators();

    /**
     * @brief Переключает страницы stackedWidget и управляет видеопотоком.
     * @param fromPage Текущая страница (не используется, но оставлен для
     * симметрии).
     * @param toPage Целевая страница.
     *
     * При переключении на страницу камеры включает видеопоток, иначе отключает.
     */
    void makePageSwitch(QWidget *fromPage, QWidget *toPage);

    Ui::HalsWindow *ui;  //!< Объект пользовательского интерфейса.
    QString m_title;  //!< Заголовок окна (имя программы + версия).
    VirtualKeyboard
        *m_keyboard;  //!< Виртуальная клавиатура для сенсорного ввода.
    QStringList m_availablePresets;  //!< Список доступных пресетов настроек.
    QSettings
        *m_settings;  //!< Объект для работы с настройками приложения (INI).
    qreal m_touchStartPos;  //!< Начальная координата X при сенсорном касании
                            //!< (для свайпа).

    HalsFacade *m_facade;  //!< Фасад, связывающий GUI с бизнес-логикой.
    QTimer *m_updatingTimer;  //!< Таймер для обновления времени (1 секунда).

    StatusIndicator *m_usbIndicator;  //!< Индикатор статуса USB-накопителя.
    StatusIndicator *m_hsIndicator;  //!< Индикатор гиперспектральной камеры.
    StatusIndicator *m_ocIndicator;  //!< Индикатор обзорной камеры.
    StatusIndicator *m_sunIndicator;  //!< Индикатор датчика освещённости.
    StatusIndicator
        *m_missionIndicator;  //!< Индикатор загрузки полётного задания.

    QRect m_hsFovRect;  //!< Прямоугольник поля зрения спектрометра на обзорном
                        //!< изображении.
};
#endif  // HALSWINDOW_H
