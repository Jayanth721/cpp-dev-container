#include <iostream>
#include <vector>
#include <memory>
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

    lock_free_queue() {
        auto dummy = new node(-1);

        head.store(dummy);
        tail.store(dummy);
    }

    void enqueue(T value) {
        node* new_node = new node(value);

        while (true) {
            node* curr_tail = this->tail.load(std::memory_order_acquire);
            node* tail_next = curr_tail->next;

            if (curr_tail->next.compare_exchange_strong(
                tail_next, new_node, std::memory_order_release)) {
                    this->tail = new_node;
                    break;
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
    node* curr_hd = this->head.load(std::memory_order_relaxed);
    while (curr_hd != nullptr)
    {
        std::cout << *curr_hd->data << std::endl;
        curr_hd = curr_hd->next.load(std::memory_order_relaxed);
    }
  }
};

int main() {
    lock_free_queue<int> lfq;
    std::thread t1([&]{
        for (int i =0; i<10; ++i) {
            lfq.enqueue(i);
        }
    });

    std::thread t2([&]{
        for (int i =10; i<20; ++i) {
            lfq.enqueue(i);
        }
    });

    // std::thread t2([&]{
    //     for (int i =0; i<100; ++i) {
    //         lfq.dequeue();
    //     }
    // });

    std::this_thread::sleep_for(std::chrono::seconds(3));
    std::thread t3([&]{
        lfq.print_queue();
    });

    t1.join();
    // t2.join();
    t3.join();
    return 0;
}
