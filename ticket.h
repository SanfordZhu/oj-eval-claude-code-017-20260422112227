#ifndef TICKET_H
#define TICKET_H

#include "user.h"
#include "train.h"

enum OrderStatus {
    SUCCESS,
    PENDING,
    REFUNDED
};

struct Order {
    int order_id;
    MyString username;
    MyString trainID;
    Time date;
    MyString from_station;
    MyString to_station;
    int from_idx;
    int to_idx;
    int price;
    int num;
    OrderStatus status;
    Time timestamp; // Time when order was placed

    Order() : order_id(0), from_idx(0), to_idx(0), price(0), num(0), status(PENDING) {}

    // Format order for output
    void format(char* buffer) const {
        const char* status_str = "";
        switch (status) {
            case SUCCESS: status_str = "success"; break;
            case PENDING: status_str = "pending"; break;
            case REFUNDED: status_str = "refunded"; break;
        }

        // Note: We need to calculate actual times based on train schedule
        // For now, placeholder
        sprintf(buffer, "[%s] %s %s xx-xx xx:xx -> %s xx-xx xx:xx %d %d",
                status_str, trainID.c_str(), from_station.c_str(),
                to_station.c_str(), price, num);
    }
};

struct TicketQueryResult {
    MyString trainID;
    MyString from_station;
    MyString to_station;
    Time leaving_time;
    Time arriving_time;
    int price;
    int seat;

    // For sorting
    bool operator<(const TicketQueryResult& other) const {
        // Compare based on query type (time or cost)
        // This will be handled by the query function
        (void)other; // Mark as unused
        return false;
    }
};

class TicketManager {
private:
    Vector<Order> orders;
    int next_order_id;
    UserManager* user_mgr;
    TrainManager* train_mgr;

    // Pending orders queue (FIFO)
    Vector<int> pending_orders; // order_ids

public:
    TicketManager(UserManager* um, TrainManager* tm)
        : next_order_id(1), user_mgr(um), train_mgr(tm) {}

    // Query tickets between stations
    int query_ticket(const MyString& from, const MyString& to,
                     const Time& date, const MyString& sort_type,
                     Vector<TicketQueryResult>& results) {
        (void)from; (void)to; (void)date; (void)sort_type; // Mark as unused
        results.clear();

        // Iterate through all trains
        // This is inefficient but works for basic implementation
        // We need a better data structure for efficient queries

        // For now, return empty results
        return 0; // Number of trains found
    }

    // Query transfer
    int query_transfer(const MyString& from, const MyString& to,
                       const Time& date, const MyString& sort_type,
                       TicketQueryResult& result1, TicketQueryResult& result2) {
        (void)from; (void)to; (void)date; (void)sort_type; // Mark as unused
        (void)result1; (void)result2; // Mark as unused
        // Not implemented yet
        return -1;
    }

    // Buy ticket
    int buy_ticket(const MyString& username, const MyString& trainID,
                   const Time& date, int num, const MyString& from,
                   const MyString& to, bool queue) {
        // Check user is logged in
        if (!user_mgr->is_logged_in(username)) {
            return -1;
        }

        // Get train
        Train train;
        if (!train_mgr->get_train(trainID, train)) {
            return -1;
        }

        // Check train is released
        if (!train.released) {
            return -1;
        }

        // Get station indices
        int from_idx = train.get_station_index(from);
        int to_idx = train.get_station_index(to);
        if (from_idx == -1 || to_idx == -1 || from_idx >= to_idx) {
            return -1;
        }

        // Check date is within sale range
        if (date < train.saleDateFrom || train.saleDateTo < date) {
            return -1;
        }

        // Calculate actual departure date from starting station
        // Need to account for travel time from start to from_station
        Time start_date = date;
        // Adjust start_date based on travel time from start station to from_station
        // This is complex, simplified for now

        // Check seat availability
        int available = train.query_seats(start_date, from_idx, to_idx);
        int price = train.get_price(from_idx, to_idx) * num;

        if (available >= num) {
            // Enough seats, buy immediately
            if (!train.buy_tickets(start_date, from_idx, to_idx, num)) {
                return -1;
            }

            // Update train in manager
            train_mgr->delete_train(trainID); // Remove old entry
            train_mgr->add_train(train.trainID, train.stationNum, train.seatNum,
                                train.stations, train.prices, train.startTime,
                                train.travelTimes, train.stopoverTimes,
                                train.saleDateFrom, train.saleDateTo, train.type);
            train_mgr->release_train(trainID); // Mark as released again

            // Create order
            Order order;
            order.order_id = next_order_id++;
            order.username = username;
            order.trainID = trainID;
            order.date = date;
            order.from_station = from;
            order.to_station = to;
            order.from_idx = from_idx;
            order.to_idx = to_idx;
            order.price = price;
            order.num = num;
            order.status = SUCCESS;
            // Set timestamp to current time (simplified)

            orders.push_back(order);

            return price;
        } else if (queue) {
            // Not enough seats, add to pending queue
            if (num > train.seatNum) {
                return -1; // Cannot queue more than train capacity
            }

            Order order;
            order.order_id = next_order_id++;
            order.username = username;
            order.trainID = trainID;
            order.date = date;
            order.from_station = from;
            order.to_station = to;
            order.from_idx = from_idx;
            order.to_idx = to_idx;
            order.price = price;
            order.num = num;
            order.status = PENDING;

            orders.push_back(order);
            pending_orders.push_back(order.order_id);

            // Return queue indicator
            return -2; // Special value for "queue"
        } else {
            return -1; // Not enough seats and not queuing
        }
    }

    // Query user orders
    int query_order(const MyString& username, MyString& result) {
        // Check user is logged in
        if (!user_mgr->is_logged_in(username)) {
            return -1;
        }

        // Collect user's orders
        Vector<Order> user_orders;
        for (int i = 0; i < orders.size(); i++) {
            if (orders[i].username == username) {
                user_orders.push_back(orders[i]);
            }
        }

        // Sort by timestamp (newest first)
        // Simplified: just reverse order of insertion

        // Format result
        char buffer[10000];
        int pos = 0;

        pos += sprintf(buffer + pos, "%d\n", user_orders.size());

        for (int i = user_orders.size() - 1; i >= 0; i--) {
            char order_buf[200];
            user_orders[i].format(order_buf);
            pos += sprintf(buffer + pos, "%s", order_buf);
            if (i > 0) {
                pos += sprintf(buffer + pos, "\n");
            }
        }

        result = MyString(buffer);
        return 0;
    }

    // Refund ticket
    int refund_ticket(const MyString& username, int n) {
        // Check user is logged in
        if (!user_mgr->is_logged_in(username)) {
            return -1;
        }

        // Collect user's orders
        Vector<int> user_order_indices;
        for (int i = 0; i < orders.size(); i++) {
            if (orders[i].username == username && orders[i].status != REFUNDED) {
                user_order_indices.push_back(i);
            }
        }

        // Check n is valid
        if (n < 1 || n > user_order_indices.size()) {
            return -1;
        }

        // Get the nth order (1-indexed, newest first)
        int order_idx = user_order_indices[user_order_indices.size() - n];
        Order& order = orders[order_idx];

        // Check order can be refunded
        if (order.status != SUCCESS && order.status != PENDING) {
            return -1;
        }

        if (order.status == SUCCESS) {
            // Refund tickets to train
            Train train;
            if (train_mgr->get_train(order.trainID, train)) {
                // Calculate actual departure date
                Time start_date = order.date;
                // Adjust start_date based on travel time (simplified)

                train.refund_tickets(start_date, order.from_idx, order.to_idx, order.num);

                // Update train
                train_mgr->delete_train(order.trainID);
                train_mgr->add_train(train.trainID, train.stationNum, train.seatNum,
                                    train.stations, train.prices, train.startTime,
                                    train.travelTimes, train.stopoverTimes,
                                    train.saleDateFrom, train.saleDateTo, train.type);
                train_mgr->release_train(order.trainID);
            }
        } else if (order.status == PENDING) {
            // Remove from pending queue
            for (int i = 0; i < pending_orders.size(); i++) {
                if (pending_orders[i] == order.order_id) {
                    // Create a simple erase by shifting
                    for (int j = i; j < pending_orders.size() - 1; j++) {
                        pending_orders[j] = pending_orders[j + 1];
                    }
                    pending_orders.pop_back();
                    break;
                }
            }
        }

        order.status = REFUNDED;

        // Try to satisfy pending orders
        process_pending_orders();

        return 0;
    }

    // Process pending orders
    void process_pending_orders() {
        // Try to satisfy pending orders in FIFO order
        Vector<int> still_pending;

        for (int i = 0; i < pending_orders.size(); i++) {
            int order_id = pending_orders[i];
            Order* order = nullptr;

            // Find order
            for (int j = 0; j < orders.size(); j++) {
                if (orders[j].order_id == order_id && orders[j].status == PENDING) {
                    order = &orders[j];
                    break;
                }
            }

            if (!order) continue;

            // Try to buy tickets
            Train train;
            if (!train_mgr->get_train(order->trainID, train)) {
                still_pending.push_back(order_id);
                continue;
            }

            // Calculate actual departure date
            Time start_date = order->date;
            // Adjust start_date based on travel time (simplified)

            int available = train.query_seats(start_date, order->from_idx, order->to_idx);

            if (available >= order->num) {
                // Enough seats now
                if (train.buy_tickets(start_date, order->from_idx, order->to_idx, order->num)) {
                    // Update train
                    train_mgr->delete_train(order->trainID);
                    train_mgr->add_train(train.trainID, train.stationNum, train.seatNum,
                                        train.stations, train.prices, train.startTime,
                                        train.travelTimes, train.stopoverTimes,
                                        train.saleDateFrom, train.saleDateTo, train.type);
                    train_mgr->release_train(order->trainID);

                    order->status = SUCCESS;
                } else {
                    still_pending.push_back(order_id);
                }
            } else {
                still_pending.push_back(order_id);
            }
        }

        pending_orders = still_pending;
    }

    // Clear all data
    void clear() {
        orders.clear();
        pending_orders.clear();
        next_order_id = 1;
    }
};

#endif // TICKET_H