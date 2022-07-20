#pragma once
#include "Definitions.h"
#include "esp_http_client.h"

enum class States
{
    Default,
    Pending,
    Running,
    Paused,
    Stopped,
    Finished
};

class Orders
{
public:
    bool retrieveOrders();
    bool retrieveOperations(const size_t &);

    bool startOperation(const size_t &, const size_t &);
    bool stopOperation(const size_t &, const size_t &);
    bool pauseOperation(const size_t &, const size_t &);

    std::optional<std::string_view> getOrderTitle(const size_t &);
    std::optional<std::string_view> getOrderID(const size_t &);
    std::optional<States> getOrderState(const size_t &);
    std::optional<size_t> getFirstOrder(const States &);
    size_t getOrderQuantity();
    void setOrderState(const size_t &, const States &);

    std::optional<std::string_view> getOperationTitle(const size_t &, const size_t &);
    std::optional<States> getOperationState(const size_t &, const size_t &);
    std::optional<size_t> getFirstOperation(const States &, const size_t &);
    std::optional<size_t> getSelectedOperation(const size_t &);
    size_t getOperationQuantity(const size_t &);
    void setOperationState(const size_t &, const size_t &, const States &);

    void setSelectedOperation(const size_t &, const size_t &);
    void incrementOperation(const size_t &);
    void decrementOperation(const size_t &);

public:
    size_t selectedOrder = 0;

private:
    static States stringToState(const char *);
    char *performRetrieveHTTP(const char *, int &);
    bool performOperationStateHTTP(const char *, const char *, const int &);

private:
    struct OperationsList
    {
        std::string title;
        std::string id;
        States state;

        OperationsList(const char *Title, const char *ID, const char *State) : title(Title), id(ID)
        {
            state = stringToState(State);
        }
    };
    struct OrdersList
    {
        std::string title;
        std::string orderID;
        std::string id;
        States state;
        std::vector<OperationsList> operations;
        size_t selectedOperation;

        OrdersList(const char *Title, const char *OrderID, const char *ID, const char *State) : title(Title), orderID(OrderID), id(ID)
        {
            state = stringToState(State);
            selectedOperation = 0;
        }
    };
    std::vector<OrdersList> m_Orders;
    std::string m_OrderURL = "https://apidev.bluestarplanning.com/Planning/Queries/Production/HMIDevices/GetProductionHMIDeviceLiveOperationGroups?WorkstationId=";
    std::string m_OperationURL = "https://apidev.bluestarplanning.com/Planning/Queries/Production/HMIDevices/GetProductionHMIDeviceLiveOperations?LiveOperationGroupId=";
};

extern Orders orders;