#pragma once

// this is all largely TODO

class Component {
  public:
    ~Component() = default;

    // GENERAL COMPONENT INTERFACE
    virtual void start() = 0; 

  private:


  protected:
    Component() { 

    }
};