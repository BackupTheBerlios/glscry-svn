#ifndef TRIAGARA_TEST_H
#define TRIAGARA_TEST_H


#include <string>
#include "Base.h"
#include "StateSet.h"
#include "Timer.h"


#define TRIAGARA_BEGIN_RESULT_DESCS()                                   \
    static void getClassResultDescs(std::vector<ResultDesc>& descs) {

#define TRIAGARA_RESULT_DESC(name, units)               \
    descs.push_back(ResultDesc((name), (units)));

#define TRIAGARA_END_RESULT_DESCS()                             \
    }                                                           \
    void getResultDescs(std::vector<ResultDesc>& descs) {       \
        getClassResultDescs(descs);                             \
    }


TRIAGARA_BEGIN_NAMESPACE


struct ResultDesc {
    ResultDesc(const std::string& name_, const std::string units_) {
        name  = name_;
        units = units_;
    }

    std::string name;
    std::string units;
};


class ResultSet {
public:
    typedef double T;

    ResultSet(size_t size) {
        _results.resize(size);
    }

    size_t size() const {
        return _results.size();
    }

    T& operator[](size_t i) {
        assert(i < size());
        return _results[i];
    }

    const T& operator[](size_t i) const {
        assert(i < size());
        return _results[i];
    }

    void normalize(T time) {
        for (size_t i = 0; i < size(); ++i) {
            _results[i] /= time;
        }
    }

private:
    std::vector<T> _results;
};


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

    ResultSet run(float runFor) {
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
        } sentry__(this);

        std::vector<ResultDesc> descs;
        getResultDescs(descs);
        ResultSet results(descs.size());

        // Don't bother timing setup and teardown.
        glFinish();

        Timer timer;
        while (timer.elapsed() < runFor) {
            iterate(results);
            pumpMessages();
        }
        glFinish();

        // timer.elapsed() is evaluated before RunSentry is destroyed.
        results.normalize(timer.elapsed());

        return results;
    }

    

    // Overridden methods.

    virtual const char* name() const = 0;

    virtual bool supported() const { return true; }

    virtual void getResultDescs(std::vector<ResultDesc>& descs) = 0;

    virtual void setup()    { }
    virtual void iterate(ResultSet& results) = 0;
    virtual void teardown() { }

private:
    StateSet* _stateSet;
};

TRIAGARA_END_NAMESPACE


#endif
