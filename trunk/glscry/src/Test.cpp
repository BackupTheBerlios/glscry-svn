#include <boost/python.hpp>
#include "Context.h"
#include "Test.h"
using namespace boost;
using namespace boost::python;


namespace scry {

    // Handles calling teardown() on exit in an exception-safe way.
    class RunSentry {
        Test* _test;
    public:
        RunSentry(Test* test): _test(test) {
            _test->getStateSet().apply();
            glPushMatrix();
            glLoadMatrixf(_test->getTransform().getData());
            _test->setup();
        }
        ~RunSentry() {
            try {
                _test->teardown();
                glPopMatrix();
                _test->getStateSet().reset();
            }
            catch (const std::exception& /*e*/) {
                // nothing
            }
        }
    };


    void Test::bind() {
        class_<Test, TestPtr, noncopyable>("Test", no_init)
            .def("setState",     &Test::setState)
            .def("setTransform", &Test::setTransform)
            .def("addAction",    &Test::addAction)
            ;
    }


    ResultSet Test::run(float runFor) {
        // Handles calling teardown() on exit in an exception-safe way.
        RunSentry sentry__(this);

        std::vector<ResultDesc> descs;
        getResultDescs(descs);
        ResultSet results(descs.size());

        // Don't bother timing setup and teardown.
        glFinish();

        Timer timer;
        while (timer.elapsed() < runFor) {
            for (size_t i = 0; i < _actionList.size(); ++i) {
                _actionList[i]->execute();
            }
            iterate(results);
            pumpMessages();
        }
        glFinish();

        // timer.elapsed() is evaluated before RunSentry is destroyed.
        results.normalize(timer.elapsed());

        return results;
    }

}
