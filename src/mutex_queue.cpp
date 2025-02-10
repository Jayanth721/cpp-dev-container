#include <iostream>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>

template <typename T>
class lock_free_mutex_queue {
    private:
    std::mutex q_mutex;
    struct node {
        std::shared_ptr<T> data;
        node* next;

        node(T init_val): next(nullptr),
        data(std::make_shared<T>(init_val)) {
        }
    };

    node* head;
    node* tail;

    public:
    lock_free_mutex_queue():
    head(nullptr),
    tail(nullptr) {
    }

    void enqueue(T data) {
        auto new_node = new node(data);
        std::lock_guard<std::mutex> guard(q_mutex);
        if (tail == nullptr){
            head = tail = new_node;
            return;
        } else {
            tail->next = new_node;
            tail = new_node;
        }
    }

    std::shared_ptr<T> dequeue() {
        std::lock_guard<std::mutex> guard(q_mutex);

        if (head == nullptr)
        return nullptr;

        auto ret_value = head->data;
        auto old_head = head;
        head = head->next;
        if (!head)
            tail = nullptr;

        delete old_head;
        return ret_value;
    }

    void print_queue() {
        std::lock_guard<std::mutex> guard(q_mutex);
        auto head_tmp = head;
        while (head_tmp != nullptr) {
            std::cout << *head_tmp->data << std::endl;
            head_tmp = head_tmp->next;
        }
    }

};

int main() {
    lock_free_mutex_queue<int> lfmq;
    std::thread t1([&]{
        for (int i =0; i<10; ++i) {
            lfmq.enqueue(i);
        }
    });

    std::thread t2([&]{
        for (int i =10; i<20; ++i) {
            lfmq.enqueue(i);
        }
    });

    // std::thread t2([&]{
    //     for (int i =0; i<100; ++i) {
    //         lfq.dequeue();
    //     }
    // });

    std::this_thread::sleep_for(std::chrono::seconds(3));
    std::thread t3([&]{
        lfmq.print_queue();
    });

    t1.join();
    t2.join();
    t3.join();
    return 0;
}