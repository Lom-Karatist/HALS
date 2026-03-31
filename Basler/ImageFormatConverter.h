#ifndef IMAGEFORMATCONVERTER_H
#define IMAGEFORMATCONVERTER_H

#include <QImage>
#include <QObject>

#include "Types.h"

class ImageFormatConverter : public QObject {
    Q_OBJECT
public:
    enum class HeatmapPalette {
        Default,  // синий -> зелёный -> красный
        Plasma,   // тёмно-синий -> пурпурный -> жёлтый
        Viridis,  // фиолетовый -> синий -> зелёный -> жёлтый
        Cividis,   //
        Grayscale  // чёрный -> белый
    };

    explicit ImageFormatConverter(QObject *parent = nullptr);

    /**
     * @brief Преобразовать данные гиперспектрометра в тепловую карту.
     *
     * Для каждого спектра (строки исходного изображения) вычисляет среднее
     * значение в заданном количестве поддиапазонов по горизонтали (длины волн).
     * Результат представляет собой изображение, где по горизонтали отложены
     * спектры, по вертикали — поддиапазоны, а цвет пикселя отражает среднюю
     * яркость.
     *
     * @param data Байтовый массив с данными.
     * @param width Ширина исходного изображения (количество длин волн).
     * @param height Высота исходного изображения (количество спектров).
     * @param pixelFormat Формат пикселей (Mono8, Mono12, Mono12p).
     * @param numBands Количество поддиапазонов (по умолчанию 5).
     * @return Тепловая карта размерами (height x numBands), или
     * null-изображение при ошибке.
     */
    static QImage convertToHeatmapImage(
        const QByteArray &data, int width, int height, int pixelFormat,
        int numBands = 20, HeatmapPalette palette = HeatmapPalette::Viridis);
    /**
     * @brief Преобразовать значение яркости в цвет для тепловой карты.
     * @param value Яркость (0..maxVal).
     * @param maxVal Максимальная яркость во всей матрице (для нормализации).
     * @return QRgb цвет.
     */
    static QRgb getHeatmapColor(
        double value, double maxVal,
        HeatmapPalette palette = HeatmapPalette::Viridis);

    /**
     * @brief Преобразовать сырые данные камеры в QImage.
     * @param data Байтовый массив.
     * @param width Ширина.
     * @param height Высота.
     * @param pixelFormat Формат пикселей.
     * @return QImage, скопированная из данных, или null-изображение в случае
     * ошибки.
     *
     * Статический метод, используется как внутри класса, так и в CameraManager.
     */
    static QImage convertToQImage(const QByteArray &data, int width, int height,
                                  int pixelFormat);

private:
    /**
     * @brief Распаковать данные формата Mono12p в массив 16-битных значений.
     * @param data Исходные упакованные данные (3 байта на 2 пикселя).
     * @param numPixels Количество пикселей.
     * @return Вектор 16-битных значений (младшие 12 бит — данные).
     */
    static QVector<quint16> unpackMono12p(const QByteArray &data,
                                          int numPixels);
    /**
     * @brief Преобразует сырые данные в массив 16-битных значений пикселей.
     * @param data Байтовый массив.
     * @param width Ширина.
     * @param height Высота.
     * @param pixelFormat Формат пикселей.
     * @return Вектор значений (0..65535, для Mono8 — 0..255, для Mono12 —
     * 0..4095).
     */
    static QVector<quint16> getPixelValues(const QByteArray &data, int width,
                                           int height, int pixelFormat);
signals:
};

#endif  // IMAGEFORMATCONVERTER_H
