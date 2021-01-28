// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <assert.h>
#include <bitset>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_set>

#include "CoreTypes.hpp"
#include "QuantumApi_I.hpp"
#include "SimFactory.hpp"
#include "SimulatorStub.hpp"
#include "context.hpp"
#include "qirTypes.hpp"
#include "quantum__rt.hpp"

#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"

// Used by a couple test simulators. Catch's REQUIRE macro doesn't deal well with static class members so making it
// into a global constant.
constexpr int RELEASED = -1;

using namespace Microsoft::Quantum;
using namespace std;

// Can manually add calls to DebugLog in the ll files for debugging.
extern "C" void DebugLog(int64_t value)
{
    std::cout << value << std::endl;
}
extern "C" void DebugLogPtr(char* value)
{
    std::cout << (const void*)value << std::endl;
}

/*
Forward declared `extern "C"` methods below are implemented in the *.ll files. Most of the files are generated by Q#
compiler, in which case the corresponding *.qs file was used as a source. Some files might have been authored manually
or manually edited.

To update the *.ll files to a newer version:
- enlist and build qsharp-compiler
- find <location1>\qsc.exe and <location2>\QirCore.qs, <location2>\QirTarget.qs built files
- [if different] copy QirCore.qs and QirTarget.qs into the "compiler" folder
- run: qsc.exe build --qir s --build-exe --input name.qs compiler\qircore.qs compiler\qirtarget.qs --proj name
- the generated file name.ll will be placed into `s` folder
*/

// The function replaces array[index] with value, then creates a new array that consists of every other element up to
// index (starting from index backwards) and every element from index to the end. It returns the sum of elements in this
// new array
extern "C" int64_t Microsoft__Quantum__Testing__QIR__Test_Arrays( // NOLINT
    int64_t count,
    int64_t* array,
    int64_t index,
    int64_t val);
TEST_CASE("QIR: Using 1D arrays", "[qir]")
{
    // re-enable tracking when https://github.com/microsoft/qsharp-compiler/issues/844 is fixed
    QirContextScope qirctx(nullptr, false /*trackAllocatedObjects*/);

    constexpr int64_t n = 5;
    int64_t values[n] = {0, 1, 2, 3, 4};

    int64_t res = Microsoft__Quantum__Testing__QIR__Test_Arrays(n, values, 2, 42);
    REQUIRE(res == (0 + 42) + (42 + 3 + 4));
}

extern "C" bool Microsoft__Quantum__Testing__QIR__Test_Qubit_Result_Management__body(); // NOLINT
struct QubitsResultsTestSimulator : public Microsoft::Quantum::SimulatorStub
{
    // no intelligent reuse, we just want to check that QIR releases all qubits
    vector<int> qubits;           // released, or |0>, or |1> states (no entanglement allowed)
    vector<int> results = {0, 1}; // released, or Zero(0) or One(1)

    int GetQubitId(Qubit qubit) const
    {
        const int id = static_cast<int>(reinterpret_cast<int64_t>(qubit));
        REQUIRE(id >= 0);
        REQUIRE(id < this->qubits.size());

        return id;
    }

    int GetResultId(Result result) const
    {
        const int id = static_cast<int>(reinterpret_cast<int64_t>(result));
        REQUIRE(id >= 0);
        REQUIRE(id < this->results.size());

        return id;
    }

    Qubit AllocateQubit() override
    {
        qubits.push_back(0);
        return reinterpret_cast<Qubit>(this->qubits.size() - 1);
    }

    void ReleaseQubit(Qubit qubit) override
    {
        const int id = GetQubitId(qubit);
        REQUIRE(this->qubits[id] != RELEASED); // no double-release
        this->qubits[id] = RELEASED;
    }

    void X(Qubit qubit) override
    {
        const int id = GetQubitId(qubit);
        REQUIRE(this->qubits[id] != RELEASED); // the qubit must be alive
        this->qubits[id] = 1 - this->qubits[id];
    }

    Result M(Qubit qubit) override
    {
        const int id = GetQubitId(qubit);
        REQUIRE(this->qubits[id] != RELEASED); // the qubit must be alive
        this->results.push_back(this->qubits[id]);
        return reinterpret_cast<Result>(this->results.size() - 1);
    }

    bool AreEqualResults(Result r1, Result r2) override
    {
        int i1 = GetResultId(r1);
        int i2 = GetResultId(r2);
        REQUIRE(this->results[i1] != RELEASED);
        REQUIRE(this->results[i2] != RELEASED);

        return (results[i1] == results[i2]);
    }

    void ReleaseResult(Result result) override
    {
        int i = GetResultId(result);
        REQUIRE(this->results[i] != RELEASED); // no double release
        this->results[i] = RELEASED;
    }

    Result UseZero() override
    {
        return reinterpret_cast<Result>(0);
    }

    Result UseOne() override
    {
        return reinterpret_cast<Result>(1);
    }
};
TEST_CASE("QIR: allocating and releasing qubits and results", "[qir]")
{
    unique_ptr<QubitsResultsTestSimulator> sim = make_unique<QubitsResultsTestSimulator>();
    QirContextScope qirctx(sim.get(), true /*trackAllocatedObjects*/);

    int64_t res = Microsoft__Quantum__Testing__QIR__Test_Qubit_Result_Management__body();
    REQUIRE(res);

    // check that all qubits have been released
    for (size_t id = 0; id < sim->qubits.size(); id++)
    {
        INFO(std::string("unreleased qubit: ") + std::to_string(id));
        CHECK(sim->qubits[id] == RELEASED);
    }

    // check that all results, allocated by measurements have been released
    // TODO: enable after https://github.com/microsoft/qsharp-compiler/issues/780 is fixed
    // for (size_t id = 2; id < sim->results.size(); id++)
    // {
    //     INFO(std::string("unreleased results: ") + std::to_string(id));
    //     CHECK(sim->results[id] == RELEASED);
    // }
}

#ifdef _WIN32
// A non-sensical function that creates a 3D array with given dimensions, then projects on the index = 1 of the
// second dimension and returns a function of the sizes of the dimensions of the projection and a the provided value,
// that is written to the original array at [1,1,1] and then retrieved from [1,1].
// Thus, all three dimensions must be at least 2.
extern "C" int64_t TestMultidimArrays(char value, int64_t dim0, int64_t dim1, int64_t dim2);
TEST_CASE("QIR: multidimensional arrays", "[qir]")
{
    QirContextScope qirctx(nullptr, true /*trackAllocatedObjects*/);

    REQUIRE(42 + (2 + 8) / 2 == TestMultidimArrays(42, 2, 4, 8));
    REQUIRE(17 + (3 + 7) / 2 == TestMultidimArrays(17, 3, 5, 7));
}
#else // not _WIN32
// TODO: The bridge for variadic functions is broken on Linux!
#endif

// Manually authored QIR to test dumping range [0..2..6] into string and then raising a failure with it
extern "C" void TestFailWithRangeString(int64_t start, int64_t step, int64_t end);
TEST_CASE("QIR: Report range in a failure message", "[qir]")
{
    QirContextScope qirctx(nullptr, true /*trackAllocatedObjects*/);

    bool failed = false;
    try
    {
        TestFailWithRangeString(0, 5, 42);
    }
    catch (const std::exception& e)
    {
        failed = true;
        REQUIRE(std::string(e.what()) == "0..5..42");
    }
    REQUIRE(failed);
}

// TestPartials subtracts the second argument from the first and returns the result.
extern "C" int64_t Microsoft__Quantum__Testing__QIR__TestPartials__body(int64_t, int64_t); // NOLINT
TEST_CASE("QIR: Partial application of a callable", "[qir]")
{
    QirContextScope qirctx(nullptr, true /*trackAllocatedObjects*/);

    const int64_t res = Microsoft__Quantum__Testing__QIR__TestPartials__body(42, 17);
    REQUIRE(res == 42 - 17);
}

// The Microsoft__Quantum__Testing__QIR__TestControlled__body tests needs proper semantics of X and M, and nothing else.
// The validation is done inside the test and it would return an error code in case of failure.
struct FunctorsTestSimulator : public Microsoft::Quantum::SimulatorStub
{
    std::vector<int> qubits;

    int GetQubitId(Qubit qubit) const
    {
        const int id = static_cast<int>(reinterpret_cast<int64_t>(qubit));
        REQUIRE(id >= 0);
        REQUIRE(id < this->qubits.size());
        return id;
    }

    Qubit AllocateQubit() override
    {
        this->qubits.push_back(0);
        return reinterpret_cast<Qubit>(this->qubits.size() - 1);
    }

    void ReleaseQubit(Qubit qubit) override
    {
        const int id = GetQubitId(qubit);
        REQUIRE(this->qubits[id] != RELEASED);
        this->qubits[id] = RELEASED;
    }

    void X(Qubit qubit) override
    {
        const int id = GetQubitId(qubit);
        REQUIRE(this->qubits[id] != RELEASED); // the qubit must be alive
        this->qubits[id] = 1 - this->qubits[id];
    }

    void ControlledX(long numControls, Qubit controls[], Qubit qubit) override
    {
        for (long i = 0; i < numControls; i++)
        {
            const int id = GetQubitId(controls[i]);
            REQUIRE(this->qubits[id] != RELEASED);
            if (this->qubits[id] == 0)
            {
                return;
            }
        }
        X(qubit);
    }

    Result M(Qubit qubit) override
    {
        const int id = GetQubitId(qubit);
        REQUIRE(this->qubits[id] != RELEASED);
        return reinterpret_cast<Result>(this->qubits[id]);
    }

    bool AreEqualResults(Result r1, Result r2) override
    {
        // those are bogus pointers but it's ok to compare them _as pointers_
        return (r1 == r2);
    }

    void ReleaseResult(Result result) override {} // the results aren't allocated by this test simulator

    Result UseZero() override
    {
        return reinterpret_cast<Result>(0);
    }

    Result UseOne() override
    {
        return reinterpret_cast<Result>(1);
    }
};
FunctorsTestSimulator* g_ctrqapi = nullptr;
extern "C" int64_t Microsoft__Quantum__Testing__QIR__TestControlled__body(); // NOLINT
extern "C" void __quantum__qis__k__body(Qubit q)                             // NOLINT
{
    g_ctrqapi->X(q);
}
extern "C" void __quantum__qis__k__ctl(QirArray* controls, Qubit q) // NOLINT
{
    g_ctrqapi->ControlledX(controls->count, reinterpret_cast<Qubit*>(controls->buffer), q);
}
TEST_CASE("QIR: application of nested controlled functor", "[qir]")
{
    unique_ptr<FunctorsTestSimulator> qapi = make_unique<FunctorsTestSimulator>();
    QirContextScope qirctx(qapi.get(), true /*trackAllocatedObjects*/);
    g_ctrqapi = qapi.get();

    REQUIRE(0 == Microsoft__Quantum__Testing__QIR__TestControlled__body());

    g_ctrqapi = nullptr;
}
