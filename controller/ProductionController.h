#pragma once
#include "../view/MainView.h"
#include "../view/SampleView.h"
#include "../view/OrderView.h"
#include "../view/MonitorView.h"
#include "../model/IRepository.h"
#include "../model/IOrderRepository.h"
#include "../model/Sample.h"
#include "../model/Order.h"

class ProductionController {
public:
    ProductionController(MainView& mainView,
                         SampleView& sampleView,
                         OrderView& orderView,
                         MonitorView& monitorView,
                         IRepository<Sample>& sampleRepo,
                         IOrderRepository& orderRepo);
    void run();
private:
    MainView&            mainView_;
    SampleView&          sampleView_;
    OrderView&           orderView_;
    MonitorView&         monitorView_;
    IRepository<Sample>& sampleRepo_;
    IOrderRepository&    orderRepo_;

    void runSampleMenu();
    void registerSample();
    void listSamples();
    void searchSamples();

    void runOrderApprovalMenu();
    void listReservedOrders();
    void approveOrder();
    void rejectOrder();

    void runMonitor();
    void showOrderStatus();
    void showStockStatus();
};
