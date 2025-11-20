#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>

namespace Prometheus{

    template <typename T> class DoubleEndedQueue {
    
        public:

        T head = nullptr;
        T tail = nullptr;
        uint64_t size = 0;

        void push(T newEntry){
            if(size == 0){
                head = newEntry;
                tail = newEntry;
            }else if(size == 1){
                tail = newEntry;
                head->next = newEntry;
                tail->prev = head;

            }else{
                T temp = tail;
                tail = newEntry;

                temp->next = newEntry;
                tail->prev = temp;
            }
            size++;
        }

        T pop(uint64_t index){

            if(index > size - 1){
                return nullptr;
            }else if (index == size - 1){
                return pop_back();
            }else{
                T target = head;
                for(uint64_t i=0; i<index-1; i++){
                    target = target->next;
                }

                target.prev->next = target->next;
                target.next->prev = target->prev;

                size--;
                return target;
            }
        }

        T pop_back(){

            if(size == 0){
                return nullptr;
            }

            T target = tail;

            if(size>1){
                tail = tail->prev;
                tail->next = nullptr;
            }else{
                tail = nullptr;
            }

            size--;
            return target;
        }

        T pop_front(){

            if(size == 0){
                return nullptr;
            }

            T target = head;

            if(size>1){
                head = head->next;
                head->prev = nullptr;
            }else{
                head = nullptr;
            }

            size--;
            return target;
        }

        T get(uint64_t index){
            if(index > size-1){
                return nullptr;
            }
            
            if(index == size - 1){
                return tail;
            }

            if(index == 0){
                return head;
            }

            T target = head;
            for(uint64_t i=0; i<index; i++){
                target = target->next;
            }

            return target;

        }

        DoubleEndedQueue(){}
        
    };
}