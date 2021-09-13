#pragma once

// this is all largely TODO

class EngineTool {
  public:
    ~EngineTool() = default;

    // GENERAL ENGINETOOL INTERFACE
    virtual void start() = 0; 

  private:


  protected:
    EngineTool() { 

    }
};