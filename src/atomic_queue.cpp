#include <iostream>
#include <vector>
#include <memory>
#include <chrono>
#include <thread>
#include <atomic>

template <typename T>
class lock_free_queue {
    private:
    struct node {
        std::shared_ptr<T> data;
        std::atomic<node*> next;

        node(T init_val): next(nullptr),
        data(std::make_shared<T>(init_val)) {
        }
    };

    std::atomic<node*> head;
    std::atomic<node*> tail;

    public:

    lock_free_queue():
    head(nullptr),
    tail(nullptr) {
        // auto dummy = new node(-1);

        // head.store();
        // tail.store(dummy);
    }

    void enqueue_perp(T value) {
        std::unique_ptr<node> new_node(new node(value));

        while (true) {
            node* curr_tail = this->tail.load(std::memory_order_acquire);

            if (curr_tail == nullptr) {
                // Empty queue case
                node* expected = nullptr;
                if (this->tail.compare_exchange_strong(expected, new_node.get(),
                                                       std::memory_order_acq_rel,
                                                       std::memory_order_relaxed)) {
                    this->head.store(new_node.get(), std::memory_order_release);
                    new_node.release();
                    return;
                }
            } else {
                node* next = curr_tail->next.load(std::memory_order_acquire);

                if (curr_tail == this->tail.load(std::memory_order_acquire)) {
                    if (next == nullptr) {
                        if (curr_tail->next.compare_exchange_strong(next, new_node.get(),
                                                                    std::memory_order_release,
                                                                    std::memory_order_relaxed)) {
                            this->tail.compare_exchange_strong(curr_tail, new_node.get(),
                                                               std::memory_order_release,
                                                               std::memory_order_relaxed);
                            new_node.release();
                            return;
                        }
                    } else {
                        this->tail.compare_exchange_strong(curr_tail, next,
                                                           std::memory_order_release,
                                                           std::memory_order_relaxed);
                    }
                }
            }
        }
    }

    void enqueue(T value) {
        node* new_node = new node(value);

        // if either tail or head is null, it's an empty queue
        auto curr_tail = this->tail.load();
        if (nullptr == curr_tail) {
            if (this->tail.compare_exchange_strong(curr_tail, new_node)) {
                this->head.store(new_node);
                return;
            }
        }

        while (true) {
            node* curr_tail = this->tail.load();
            node* tail_next = curr_tail->next.load();

            // this is needed to make sure, tail is upto date
            if (tail_next == nullptr) {
                if (curr_tail->next.compare_exchange_strong(
                    tail_next, new_node)) {
                        this->tail.store(new_node);
                        break;
                }
            }
        }
    }

    std::shared_ptr<T> dequeue() {
        std::shared_ptr<T> return_value = nullptr;

        //  do an infinite loop to change the head
        while (true) {
            node* current_head = this->head.load(std::memory_order_acquire);
            node* next_node = current_head->next;

            if (this->head.compare_exchange_strong(current_head, next_node,
                std::memory_order_release)) {
                    return_value.swap(next_node->data);
                    delete current_head;
                    break;
            }
        }
        return return_value;
    }

  void print_queue() {
    node* curr_hd = this->head.load();
    while (curr_hd != nullptr)
    {
        std::cout << *curr_hd->data << std::endl;
        curr_hd = curr_hd->next.load();
    }
  }
};

int main() {
    using namespace std::chrono_literals;
    lock_free_queue<int> lfq;
    std::thread t1([&]{
        for (int i = 0; i < 3; ++i) {
            std::this_thread::sleep_for(1500ms);
            lfq.enqueue_perp(i);
        }
    });

    std::thread t2([&]{
        for (int i = 10; i < 13; ++i) {
            std::this_thread::sleep_for(1500ms);
            lfq.enqueue_perp(i);
        }
    });

    // std::thread t2([&]{
    //     for (int i =0; i<100; ++i) {
    //         lfq.dequeue();
    //     }
    // });

    std::this_thread::sleep_for(std::chrono::seconds(8));
    std::thread t3([&]{
        lfq.print_queue();
    });

    t1.join();
    t2.join();
    t3.join();
    return 0;
}
