#pragma once
#include <string>
#include <vector>
#include "../model/Order.h"
#include "../model/Sample.h"

class OrderView {
public:
    virtual ~OrderView() = default;
    virtual int         inputSampleId();
    virtual std::string inputCustomerName();
    virtual int         inputQuantity();
    virtual void        showOrderRegistered(const Order& o);
    virtual void        showOrderList(const std::vector<Order>& orders,
                                     const std::vector<Sample>& samples);
    virtual void        showNoOrders();
    virtual void        showInvalidInput(const std::string& msg);
    virtual void        showComingSoon();
    virtual void        showApprovalMenu();
    virtual int         inputOrderId();
    virtual void        showConfirmed(const Order& o);
    virtual void        showSentToProduction(const Order& o);
    virtual void        showRejected(const Order& o);
};
