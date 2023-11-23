
// #define NIMATA_LOGGING
#include "../include/Nimata.hpp"
#include <iostream>
#include <thread>

int main()
{
  Nimata::Queue<int> queue;
  queue.push(1);
  queue.push(2);
  queue.push(3);
  
  while (queue)
  {
    std::cout << queue.grab() << '\n';
  }

  queue.push(4);
  
  std::cout << (queue ? "queue is not empty\n" : "queue is empty\n");
  queue.drop();
  std::cout << (queue ? "queue is not empty\n" : "queue is empty\n");

  std::thread thread_1{[&]{queue.push(5), queue.push(5), queue.push(5);}};
  std::thread thread_2{[&]{queue.push(5), queue.push(5);}};

  thread_1.join();
  thread_2.join();

  while (queue)
  {
    std::cout << queue.grab() << ' ';
  }
}