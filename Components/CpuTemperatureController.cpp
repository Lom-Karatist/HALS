#include "CpuTemperatureController.h"

#include <QDebug>

#ifdef Q_OS_WIN
#include <Wbemidl.h>
#include <comdef.h>
#include <windows.h>
#endif

CpuTemperatureController::CpuTemperatureController(QObject *parent)
    : QObject{parent} {
    m_tempTimer = new QTimer(this);
    connect(m_tempTimer, &QTimer::timeout, this,
            &CpuTemperatureController::getCpuTemperature);
    m_tempTimer->start(10000);
}

CpuTemperatureController::~CpuTemperatureController() {
    if (m_tempTimer->isActive()) m_tempTimer->stop();
}

void CpuTemperatureController::getCpuTemperature() {
#ifdef Q_OS_LINUX
    QFile file("/sys/class/thermal/thermal_zone0/temp");
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream in(&file);
        QString tempStr = in.readLine();
        file.close();
        bool ok;
        int tempInt = tempStr.toInt(&ok);
        if (ok && temp > 0) {
            emit cpuTemperatureUpdated(QString::number(tempInt / 1000));
        }
    }
#elif defined(Q_OS_WIN)
    HRESULT hres = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hres) && hres != S_FALSE) {
        emit cpuTemperatureUpdated("-1");
        return;
    }

    IWbemLocator *pLoc = nullptr;
    hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
                            IID_IWbemLocator, (LPVOID *)&pLoc);
    if (FAILED(hres)) {
        CoUninitialize();
        emit cpuTemperatureUpdated("-2");
        return;
    }

    IWbemServices *pSvc = nullptr;
    hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\WMI"), nullptr, nullptr, 0, 0, 0,
                               0, &pSvc);
    if (FAILED(hres)) {
        pLoc->Release();
        CoUninitialize();
        emit cpuTemperatureUpdated("-3");
        return;
    }

    IEnumWbemClassObject *pEnumerator = nullptr;
    hres = pSvc->ExecQuery(
        bstr_t("WQL"), bstr_t("SELECT * FROM MSAcpi_ThermalZoneTemperature"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr,
        &pEnumerator);
    if (SUCCEEDED(hres)) {
        IWbemClassObject *pclsObj = nullptr;
        ULONG uReturn = 0;
        while (pEnumerator) {
            hres = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
            if (uReturn == 0) break;

            VARIANT vtProp;
            hres = pclsObj->Get(L"CurrentTemperature", 0, &vtProp, 0, 0);
            if (SUCCEEDED(hres)) {
                int tempKelvinTenths = vtProp.intVal;
                double tempCelsius = (tempKelvinTenths / 10.0) - 273.15;
                emit cpuTemperatureUpdated(
                    QString::number(tempCelsius, 'f', 1));
                VariantClear(&vtProp);
            } else {
                emit cpuTemperatureUpdated("-4");
            }
            pclsObj->Release();
        }
        pEnumerator->Release();
    } else {
        emit cpuTemperatureUpdated("-5");
    }

    pSvc->Release();
    pLoc->Release();
    CoUninitialize();
#else
    emit cpuTemperatureUpdated("-1");

#endif
}
