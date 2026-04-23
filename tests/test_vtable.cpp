#include <iostream>
#include <string>
#include "dobby_vtable.h"

// Define a simple class hierarchy for testing
class Base {
public:
    virtual ~Base() = default;
    
    virtual int GetValue() {
        return 100;
    }
    
    virtual std::string GetString() {
        return "Base";
    }
};

class Derived : public Base {
public:
    int GetValue() override {
        return 200;
    }
    
    std::string GetString() override {
        return "Derived";
    }
    
    virtual int GetDerivedValue() {
        return 300;
    }
};

// Hook functions for Global VTable hooking
int Hooked_Derived_GetValue(Derived* self) {
    std::cout << "Hooked_Derived_GetValue called!" << std::endl;
    return 999;
}

// Global backup pointer
void* orig_Derived_GetValue = nullptr;

// Hook functions for Instance VTable hooking
int InstanceHooked_GetValue(Derived* self) {
    std::cout << "InstanceHooked_GetValue called!" << std::endl;
    return 777;
}

// Prevent devirtualization
__attribute__((noinline)) int CallGetValue(Base* obj) {
    return obj->GetValue();
}

void RunTests() {
    std::cout << "--- VTable Hooking Tests ---" << std::endl;

    Derived* d1_raw = new Derived();
    Derived* d2_raw = new Derived();
    
    Base* d1 = d1_raw;
    Base* d2 = d2_raw;

    // 1. Test Auto Math / Calculate Index
    auto idx1 = dobby::vtable::AutoCalculateIndex(&Base::GetValue);
    auto idx2 = dobby::vtable::AutoCalculateIndex(&Base::GetString);
    auto idx3 = dobby::vtable::AutoCalculateIndex(&Derived::GetDerivedValue);

    std::cout << "GetValue Index: " << (idx1 ? std::to_string(idx1.value()) : "Error") << std::endl;
    std::cout << "GetString Index: " << (idx2 ? std::to_string(idx2.value()) : "Error") << std::endl;
    std::cout << "GetDerivedValue Index: " << (idx3 ? std::to_string(idx3.value()) : "Error") << std::endl;

    // 2. Global VTable Hooking using Auto Calculate
    std::cout << "\n--- Testing Global VTable Hook ---" << std::endl;
    printf("Target hook function addr: %p\n", (void*)Hooked_Derived_GetValue);
    std::cout << "d1 GetValue (Before): " << CallGetValue(d1) << std::endl;
    std::cout << "d2 GetValue (Before): " << CallGetValue(d2) << std::endl;

    auto res = dobby::vtable::HookVirtualMethod(
        d1, 
        &Base::GetValue, 
        reinterpret_cast<void*>(Hooked_Derived_GetValue), 
        &orig_Derived_GetValue
    );

    if (res) {
        std::cout << "Global hook applied successfully." << std::endl;
    } else {
        std::cout << "Global hook failed." << std::endl;
    }

    std::cout << "d1 GetValue (After Hook): " << CallGetValue(d1) << std::endl;
    std::cout << "d2 GetValue (After Hook): " << CallGetValue(d2) << std::endl; 

    // Restore Global Hook
    if (res && orig_Derived_GetValue) {
        dobby::vtable::HookVirtualMethod(
            d1, 
            &Base::GetValue, 
            orig_Derived_GetValue, 
            nullptr
        );
        std::cout << "Global hook restored." << std::endl;
    }
    
    std::cout << "d1 GetValue (After Restore): " << CallGetValue(d1) << std::endl;

    // 3. Instance VTable Hooking (ReplaceVTable)
    std::cout << "\n--- Testing Instance VTable Hook ---" << std::endl;
    
    {
        dobby::vtable::InstanceHook instance_hook(d1, 10);
        
        auto hook_res = instance_hook.HookMethod(
            &Base::GetValue, 
            reinterpret_cast<void*>(InstanceHooked_GetValue)
        );

        if (hook_res) {
            std::cout << "Instance hook applied successfully." << std::endl;
        } else {
            std::cout << "Instance hook failed." << std::endl;
        }

        std::cout << "d1 GetValue (Instance Hooked): " << CallGetValue(d1) << std::endl; 
        std::cout << "d2 GetValue (Not Hooked): " << CallGetValue(d2) << std::endl; 
    } 

    std::cout << "d1 GetValue (After Instance Hook Destructor): " << CallGetValue(d1) << std::endl; 

    delete d1_raw;
    delete d2_raw;
}

int main() {
    RunTests();
    return 0;
}
