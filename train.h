#ifndef TRAIN_H
#define TRAIN_H

#include "utils.h"

struct StationInfo {
    MyString name;
    int price;  // Cumulative price from start
    Time arrival;
    Time departure;
    int seat;   // Available seats from this station to next

    StationInfo() : price(0), seat(0) {}
};

struct Train {
    MyString trainID;
    char type;
    int stationNum;
    int seatNum;
    Time startTime;
    Time saleDateFrom;
    Time saleDateTo;
    bool released;

    Vector<MyString> stations;
    Vector<int> prices;        // Price between stations
    Vector<int> travelTimes;   // Travel time between stations
    Vector<int> stopoverTimes; // Stopover time at stations (except first and last)

    // For each day in sale range, store seat availability
    // seats[day_index][station_index] = available seats from station to next
    Vector<Vector<int>> seats;

    Train() : type('G'), stationNum(0), seatNum(0), released(false) {}

    // Initialize seats array
    void init_seats() {
        int days = (saleDateTo.to_minutes() - saleDateFrom.to_minutes()) / (24 * 60) + 1;
        seats.clear();
        for (int i = 0; i < days; i++) {
            Vector<int> day_seats;
            for (int j = 0; j < stationNum - 1; j++) {
                day_seats.push_back(seatNum);
            }
            seats.push_back(day_seats);
        }
    }

    // Get day index from date
    int get_day_index(const Time& date) const {
        return (date.to_minutes() - saleDateFrom.to_minutes()) / (24 * 60);
    }

    // Calculate station times for a given departure date
    void calculate_times(const Time& departure_date, Vector<StationInfo>& result) const {
        result.clear();

        Time current_time = departure_date;
        current_time.hour = startTime.hour;
        current_time.minute = startTime.minute;

        int cumulative_price = 0;

        for (int i = 0; i < stationNum; i++) {
            StationInfo info;
            info.name = stations[i];
            info.price = cumulative_price;

            if (i == 0) {
                // Starting station
                info.arrival.month = info.arrival.day = info.arrival.hour = info.arrival.minute = -1;
                info.departure = current_time;
            } else {
                // Arrival time = departure from previous station + travel time
                info.arrival = current_time.add_minutes(travelTimes[i-1]);

                if (i == stationNum - 1) {
                    // Terminal station
                    info.departure.month = info.departure.day = info.departure.hour = info.departure.minute = -1;
                } else {
                    // Departure time = arrival time + stopover time
                    info.departure = info.arrival.add_minutes(stopoverTimes[i-1]);
                    current_time = info.departure;
                }
            }

            // Seat availability
            if (i < stationNum - 1 && released) {
                int day_idx = get_day_index(departure_date);
                if (day_idx >= 0 && day_idx < seats.size()) {
                    info.seat = seats[day_idx][i];
                } else {
                    info.seat = 0;
                }
            } else {
                info.seat = -1; // Mark as not applicable
            }

            // Update cumulative price for next station
            if (i < stationNum - 1) {
                cumulative_price += prices[i];
            }

            result.push_back(info);
        }
    }

    // Query available seats between stations on a specific day
    int query_seats(const Time& date, int from_idx, int to_idx) const {
        if (!released) return seatNum; // Not released yet, all seats available

        int day_idx = get_day_index(date);
        if (day_idx < 0 || day_idx >= seats.size()) return 0;

        int min_seats = seatNum;
        for (int i = from_idx; i < to_idx; i++) {
            if (seats[day_idx][i] < min_seats) {
                min_seats = seats[day_idx][i];
            }
        }
        return min_seats;
    }

    // Buy tickets
    bool buy_tickets(const Time& date, int from_idx, int to_idx, int num) {
        if (!released) return false;

        int day_idx = get_day_index(date);
        if (day_idx < 0 || day_idx >= seats.size()) return false;

        // Check if enough seats available
        for (int i = from_idx; i < to_idx; i++) {
            if (seats[day_idx][i] < num) {
                return false;
            }
        }

        // Update seats
        for (int i = from_idx; i < to_idx; i++) {
            seats[day_idx][i] -= num;
        }

        return true;
    }

    // Refund tickets
    void refund_tickets(const Time& date, int from_idx, int to_idx, int num) {
        if (!released) return;

        int day_idx = get_day_index(date);
        if (day_idx < 0 || day_idx >= seats.size()) return;

        for (int i = from_idx; i < to_idx; i++) {
            seats[day_idx][i] += num;
            if (seats[day_idx][i] > seatNum) {
                seats[day_idx][i] = seatNum;
            }
        }
    }

    // Get station index by name
    int get_station_index(const MyString& station_name) const {
        for (int i = 0; i < stationNum; i++) {
            if (stations[i] == station_name) {
                return i;
            }
        }
        return -1;
    }

    // Calculate price between stations
    int get_price(int from_idx, int to_idx) const {
        int price = 0;
        for (int i = from_idx; i < to_idx; i++) {
            price += prices[i];
        }
        return price;
    }

    // Calculate travel time between stations
    int get_travel_time(int from_idx, int to_idx) const {
        int time = 0;
        for (int i = from_idx; i < to_idx; i++) {
            time += travelTimes[i];
            if (i < to_idx - 1) {
                time += stopoverTimes[i];
            }
        }
        return time;
    }
};

class TrainManager {
private:
    HashMap<MyString, Train> trains;
    HashMap<MyString, bool> released_trains; // trainID -> released

public:
    TrainManager() {}

    // Add a new train
    int add_train(const MyString& trainID, int stationNum, int seatNum,
                  const Vector<MyString>& stations, const Vector<int>& prices,
                  const Time& startTime, const Vector<int>& travelTimes,
                  const Vector<int>& stopoverTimes, const Time& saleDateFrom,
                  const Time& saleDateTo, char type) {
        // Check if trainID already exists
        Train dummy;
        if (trains.find(trainID, dummy)) {
            return -1;
        }

        Train train;
        train.trainID = trainID;
        train.stationNum = stationNum;
        train.seatNum = seatNum;
        train.stations = stations;
        train.prices = prices;
        train.startTime = startTime;
        train.travelTimes = travelTimes;
        train.stopoverTimes = stopoverTimes;
        train.saleDateFrom = saleDateFrom;
        train.saleDateTo = saleDateTo;
        train.type = type;
        train.released = false;

        train.init_seats();

        trains.insert(trainID, train);
        return 0;
    }

    // Release a train
    int release_train(const MyString& trainID) {
        Train train;
        if (!trains.find(trainID, train)) {
            return -1;
        }

        if (train.released) {
            return -1; // Already released
        }

        train.released = true;
        trains.erase(trainID);
        trains.insert(trainID, train);
        released_trains.insert(trainID, true);

        return 0;
    }

    // Delete a train
    int delete_train(const MyString& trainID) {
        Train train;
        if (!trains.find(trainID, train)) {
            return -1;
        }

        if (train.released) {
            return -1; // Cannot delete released train
        }

        trains.erase(trainID);
        return 0;
    }

    // Query train information
    int query_train(const MyString& trainID, const Time& date, MyString& result) {
        Train train;
        if (!trains.find(trainID, train)) {
            return -1;
        }

        // Check if date is within sale range
        if (date < train.saleDateFrom || train.saleDateTo < date) {
            return -1;
        }

        Vector<StationInfo> stations;
        train.calculate_times(date, stations);

        // Format result
        char buffer[50000]; // Increased buffer size for worst case
        int pos = 0;

        // First line: trainID type
        pos += sprintf(buffer + pos, "%s %c\n", trainID.c_str(), train.type);

        // Station information
        for (int i = 0; i < train.stationNum; i++) {
            const StationInfo& info = stations[i];

            // Station name
            pos += sprintf(buffer + pos, "%s ", info.name.c_str());

            // Arrival time
            if (info.arrival.month == -1) {
                pos += sprintf(buffer + pos, "xx-xx xx:xx");
            } else {
                char time_buf[20];
                info.arrival.format_datetime(time_buf);
                pos += sprintf(buffer + pos, "%s", time_buf);
            }

            pos += sprintf(buffer + pos, " -> ");

            // Departure time
            if (info.departure.month == -1) {
                pos += sprintf(buffer + pos, "xx-xx xx:xx");
            } else {
                char time_buf[20];
                info.departure.format_datetime(time_buf);
                pos += sprintf(buffer + pos, "%s", time_buf);
            }

            // Price
            pos += sprintf(buffer + pos, " %d", info.price);

            // Seat
            if (info.seat == -1) {
                pos += sprintf(buffer + pos, " x");
            } else {
                pos += sprintf(buffer + pos, " %d", info.seat);
            }

            pos += sprintf(buffer + pos, "\n");
        }

        result = MyString(buffer);
        return 0;
    }

    // Get train by ID
    bool get_train(const MyString& trainID, Train& train) {
        return trains.find(trainID, train);
    }

    // Check if train is released
    bool is_released(const MyString& trainID) {
        bool released;
        if (released_trains.find(trainID, released)) {
            return released;
        }
        return false;
    }

    // Clear all trains
    void clear() {
        trains.clear();
        released_trains.clear();
    }
};

#endif // TRAIN_H