#ifndef _APP_H_
#define _APP_H_

class App {
public:
    static App& instance();
    void setup();
    void loop();
    
private:
    App() {} // Private constructor for singleton pattern
    App(const App&) = delete;
    App& operator=(const App&) = delete;
};

#endif // _APP_H_
