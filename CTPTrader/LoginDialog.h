#pragma once

#include <qdialog.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <mutex>
#include "LogSystem.h"
#include "json.hpp"


class CTPTrader;
class LoginDialog :public QDialog,public LogSystem
{
	Q_OBJECT
public:
    LoginDialog(CTPTrader* trader);
    ~LoginDialog() override { }
    void SetUpUi();
    void CloseDialog();
    void Join();
    void EnableBtnOK(bool flag);
    static nlohmann::json brokerInfo;
protected:
    static std::once_flag _flag;
    static bool GetBrokerInfo();
    static QStringList GetBrokerNames();
    QVBoxLayout* VlayLabels, * VlayInputs, * Vlay;
    QHBoxLayout* HlayBtns, * Hlay;
    QComboBox* combboxSelectTrader;
    QLabel* lbBrokerID, * lbUserID, * lbPassWord, * lbLog;
    QLineEdit * editUserID, * editPassWord;
    QPushButton* btnOK, * btnCancel;
    std::mutex _logMut;

    CTPTrader* _trader;

    void Log(const QString& log) override;
    void Log(const std::string& log) override;
protected slots:
    virtual void onBtnOK();
    void onBtnCancel();
signals:
    void closeDialog();
    void enableBtnOK(bool);
};

class FirstLoginDialog :public LoginDialog
{
    Q_OBJECT
public:
    FirstLoginDialog(CTPTrader* trader);
    ~FirstLoginDialog() override {}
private slots:
    void onBtnOK() override;
};