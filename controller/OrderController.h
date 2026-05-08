#pragma once
#include "../view/MainView.h"
#include "../view/OrderView.h"
#include "../model/IRepository.h"
#include "../model/IOrderRepository.h"
#include "../model/Sample.h"

class OrderController {
public:
    OrderController(MainView& mainView,
                    OrderView& orderView,
                    IRepository<Sample>& sampleRepo,
                    IOrderRepository& orderRepo);
    void run();
private:
    MainView&            mainView_;
    OrderView&           orderView_;
    IRepository<Sample>& sampleRepo_;
    IOrderRepository&    orderRepo_;

    void receiveOrder();
    void listReservedOrders();
};
