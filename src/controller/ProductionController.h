#pragma once
#include "../view/MainView.h"
#include "../view/SampleView.h"
#include "../view/OrderView.h"
#include "../view/MonitorView.h"
#include "../view/ProductionView.h"
#include "../model/IRepository.h"
#include "../model/IOrderRepository.h"
#include "../model/Sample.h"
#include "../model/Order.h"
#include "../model/ProductionEntry.h"
#include <vector>

class ProductionController {
public:
    ProductionController(MainView& mainView,
                         SampleView& sampleView,
                         OrderView& orderView,
                         MonitorView& monitorView,
                         ProductionView& productionView,
                         IRepository<Sample>& sampleRepo,
                         IOrderRepository& orderRepo);
    void run();
private:
    MainView&            mainView_;
    SampleView&          sampleView_;
    OrderView&           orderView_;
    MonitorView&         monitorView_;
    ProductionView&      productionView_;
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

    void runProductionMenu();
    void showProductionStatus();
    void showProductionQueue();
    void completeProduction();
    std::vector<ProductionEntry> buildProductionEntries(bool sortByUpdatedAt);
};
