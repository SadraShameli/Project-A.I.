#include "Orders.h"

std::optional<std::string_view> Orders::getOrderTitle(const size_t &order)
{
    if (order < m_Orders.size())
        return m_Orders[order].title;
    return {};
}
std::optional<std::string_view> Orders::getOrderID(const size_t &order)
{
    if (order < m_Orders.size())
        return m_Orders[order].orderID;
    return {};
}
std::optional<States> Orders::getOrderState(const size_t &order)
{
    if (order < m_Orders.size())
        return m_Orders[order].state;
    return {};
}
std::optional<size_t> Orders::getFirstOrder(const States &s)
{
    size_t orderSize = m_Orders.size();
    for (size_t i = 0; i < orderSize; ++i)
    {
        if (m_Orders[i].state == s)
            return i;
    }
    return {};
}
size_t Orders::getOrderQuantity()
{
    return m_Orders.size();
}
void Orders::setOrderState(const size_t &order, const States &state)
{
    if (order < m_Orders.size())
        return;
    m_Orders[order].state = state;
}

std::optional<std::string_view> Orders::getOperationTitle(const size_t &order, const size_t &operation)
{
    if (order < m_Orders.size() && operation < m_Orders[order].operations.size())
        return m_Orders[order].operations[operation].title;
    return {};
}
std::optional<States> Orders::getOperationState(const size_t &order, const size_t &operation)
{
    if (order < m_Orders.size() && operation < m_Orders[order].operations.size())
        return m_Orders[order].operations[operation].state;
    return {};
}
std::optional<size_t> Orders::getFirstOperation(const States &state, const size_t &order)
{
    size_t operationSize = m_Orders[order].operations.size();
    for (size_t i = 0; i < operationSize; ++i)
    {
        if (m_Orders[order].operations[i].state == state)
            return i;
    }
    return {};
}
std::optional<size_t> Orders::getSelectedOperation(const size_t &order)
{
    if (order < m_Orders.size())
        return m_Orders[order].selectedOperation;
    return {};
}
size_t Orders::getOperationQuantity(const size_t &order)
{
    if (order < m_Orders.size())
        return m_Orders[order].operations.size();
    return 0;
}
void Orders::setOperationState(const size_t &order, const size_t &operation, const States &state)
{
    if (order < m_Orders.size() && operation < m_Orders[order].operations.size())
        m_Orders[order].operations[operation].state = state;
}

void Orders::setSelectedOperation(const size_t &order, const size_t &operation)
{
    if (order < m_Orders.size() && operation < m_Orders[order].operations.size())
        m_Orders[order].selectedOperation = operation;
}
void Orders::incrementOperation(const size_t &order)
{
    if (order < m_Orders.size())
        ++m_Orders[order].selectedOperation;
}
void Orders::decrementOperation(const size_t &order)
{
    if (order < m_Orders.size())
        --m_Orders[order].selectedOperation;
}

States Orders::stringToState(const char *str)
{
    if (!strcmp(str, "Running"))
        return States::Running;
    else if (!strcmp(str, "Stopped"))
        return States::Stopped;
    else if (!strcmp(str, "Paused"))
        return States::Paused;
    return States::Default;
}

Orders orders;