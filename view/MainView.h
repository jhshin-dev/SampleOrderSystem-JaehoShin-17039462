#pragma once

class MainView {
public:
    virtual ~MainView() = default;
    virtual void showRoleMenu(int sampleCount, int totalStock);
    virtual void showOrderManagerMenu(int sampleCount, int totalStock);
    virtual void showProductionManagerMenu(int sampleCount, int totalStock);
    virtual void showComingSoon();
    virtual void showInvalidInput();
    virtual int  getMenuInput();
};
