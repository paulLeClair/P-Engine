#pragma once

// hoping i can just use a super-simple ID system for things that need a unique id at runtime;
    // gonna use uint64 so that we can handle tons of ID'd objects
class UniqueID {
    public: 
        using uid_t = unsigned long long;

        // credits to C++ cookbook by stephens, diggins, turkanis, cogswell for the design!
        UniqueID();

        UniqueID(const UniqueID &original);
        UniqueID &operator=(const UniqueID &original);

        const uid_t &get() {
            return id_;
        }

    private:
        uid_t id_;    

    protected:
        static uid_t nextID;
};