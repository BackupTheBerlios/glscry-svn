#ifndef TRIAGARA_TEST_H
#define TRIAGARA_TEST_H


#include "Base.h"
#include "StateSet.h"
#include "Timer.h"


TRIAGARA_BEGIN_NAMESPACE

class Test {
public:
    Test() {
        _stateSet = 0;
    }

    virtual ~Test() { }

    // Public interface.

    void setStateSet(StateSet* stateSet) {
        _stateSet = stateSet;
    }

    StateSet* getStateSet() const {
        return _stateSet;
    }

    Uint64 run(float runFor) {
        // Handles calling teardown() on exit in an exception-safe way.
        class RunSentry {
            Test* _test;
        public:
            RunSentry(Test* test): _test(test) {
                if (_test->getStateSet()) {
                    _test->getStateSet()->apply();
                }
                _test->setup();
            }
            ~RunSentry() {
                try {
                    _test->teardown();
                    if (_test->getStateSet()) {
                        _test->getStateSet()->reset();
                    }
                }
                catch (const std::exception& e) {
                    // nothing
                }
            }
        };

        Uint64 result = 0;
        RunSentry sentry__(this);
        Timer timer;
        while (timer.elapsed() < runFor) {
            result += iterate();
            pumpMessages();
        }
        glFinish();
        // timer.elapsed() is evaluated before RunSentry is destroyed.
        return Uint64(result / timer.elapsed());
    }

    // Overridden methods.

    virtual const char* name() const = 0;
    virtual const char* units() const = 0;

    virtual bool supported() const { return true; }

    virtual void   setup()    { }
    virtual Uint64 iterate() = 0;
    virtual void   teardown() { }

private:
    StateSet* _stateSet;
};

TRIAGARA_END_NAMESPACE


#endif
