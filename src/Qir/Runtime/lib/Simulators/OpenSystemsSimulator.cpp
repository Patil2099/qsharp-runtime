// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <assert.h>
#include <bitset>
#include <complex>
#include <iostream>
#include <memory>
#include <vector>

#include "SimFactory.hpp"
#include "QuantumApi_I.hpp"

using namespace std;
typedef long long int i64;

// Define the API with the open systems simulator.
extern "C"
{
    size_t init(size_t initialcapacity);
    i64 destroy(size_t sim_id);
    void dump_to_console(size_t sim_id);
    i64 x(size_t sim_id, size_t idx);
    i64 y(size_t sim_id, size_t idx);
    i64 z(size_t sim_id, size_t idx);
    i64 h(size_t sim_id, size_t idx);
    i64 s(size_t sim_id, size_t idx);
    i64 s_adj(size_t sim_id, size_t idx);
    i64 t(size_t sim_id, size_t idx);
    i64 t_adj(size_t sim_id, size_t idx);
    i64 cnot(size_t sim_id, size_t idx_control, size_t idx_target);
    i64 m(size_t sim_id, size_t idx, size_t* result_out);
    const char* lasterr();
    const char* get_noise_model(size_t sim_id);
    i64 set_noise_model(size_t sim_id, const char* new_model);
    const char* get_current_state(size_t sim_id);
}

namespace Microsoft
{
namespace Quantum
{
    // FIXME: support methods from public IDiagnostics; they currently
    //        just throw.
    class OpenSystemSimulator : public IRuntimeDriver, public IQuantumGateSet, public IDiagnostics
    {
        typedef void (*TSingleQubitGate)(size_t /*simulator id*/, size_t /*qubit id*/);
        typedef void (*TSingleQubitControlledGate)(
            unsigned /*simulator id*/,
            unsigned /*number of controls*/,
            unsigned* /*controls*/,
            unsigned /*qubit id*/);

        // QuantumSimulator defines paulis as:
        // enum Basis
        // {
        //     PauliI = 0,
        //     PauliX = 1,
        //     PauliY = 3,
        //     PauliZ = 2
        // };
        // which (surprise!) matches our definition of PauliId enum
        static inline unsigned GetBasis(PauliId pauli)
        {
            return static_cast<unsigned>(pauli);
        }

        unsigned simulatorId = -1;
        unsigned nextQubitId = 0; // the QuantumSimulator expects contiguous ids, starting from 0

        unsigned GetQubitId(Qubit qubit) const
        {
            return static_cast<unsigned>(reinterpret_cast<size_t>(qubit));
        }

        vector<unsigned> GetQubitIds(long num, Qubit* qubits) const
        {
            vector<unsigned> ids;
            ids.reserve(num);
            for (long i = 0; i < num; i++)
            {
                ids.push_back(static_cast<unsigned>(reinterpret_cast<size_t>(qubits[i])));
            }
            return ids;
        }

        // use for debugging the simulator
        void DumpState()
        {
            dump_to_console(simulatorId);
        }

      public:
        OpenSystemSimulator()
        {
            // FIXME: allow setting number of qubits.
            this->simulatorId = init(3);
        }
        ~OpenSystemSimulator()
        {
            destroy(this->simulatorId);
        }

        void GetState(TGetStateCallback callback) override
        {
            throw std::logic_error("operation_not_supported");
        }
        bool Assert(long numTargets, PauliId* bases, Qubit* targets, Result result, const char* failureMessage) override
        {
            throw std::logic_error("operation_not_supported");
        }

        bool AssertProbability(
            long numTargets,
            PauliId bases[],
            Qubit targets[],
            double probabilityOfZero,
            double precision,
            const char* failureMessage) override
        {
            throw std::logic_error("operation_not_supported");
        }

        virtual std::string QubitToString(Qubit q) override
        {
            return std::to_string(GetQubitId(q));
        }

        Qubit AllocateQubit() override
        {
            const unsigned id = this->nextQubitId;
            this->nextQubitId++;
            return reinterpret_cast<Qubit>(id);
        }

        void ReleaseQubit(Qubit q) override
        {
            // TODO
        }

        Result M(Qubit q) override
        {
            size_t result;
            m(this->simulatorId, GetQubitId(q), &result);
            return reinterpret_cast<Result>(result);
        }

        Result Measure(long numBases, PauliId bases[], long numTargets, Qubit targets[]) override
        {
            throw std::logic_error("not yet implemented");
        }

        void ReleaseResult(Result r) override {}

        ResultValue GetResultValue(Result r) override
        {
            const unsigned val = static_cast<unsigned>(reinterpret_cast<size_t>(r));
            assert(val == 0 || val == 1);
            return (val == 0) ? Result_Zero : Result_One;
        }

        Result UseZero() override
        {
            return reinterpret_cast<Result>(0);
        }

        Result UseOne() override
        {
            return reinterpret_cast<Result>(1);
        }

        bool AreEqualResults(Result r1, Result r2) override
        {
            return (r1 == r2);
        }

        void X(Qubit q) override
        {
            x(this->simulatorId, GetQubitId(q));
        }

        void ControlledX(long numControls, Qubit controls[], Qubit target) override
        {
            throw std::logic_error("not yet implemented");
        }

        void Y(Qubit q) override
        {
            y(this->simulatorId, GetQubitId(q));
        }

        void ControlledY(long numControls, Qubit controls[], Qubit target) override
        {
            throw std::logic_error("not yet implemented");
        }

        void Z(Qubit q) override
        {
            z(this->simulatorId, GetQubitId(q));
        }

        void ControlledZ(long numControls, Qubit controls[], Qubit target) override
        {
            throw std::logic_error("not yet implemented");
        }

        void H(Qubit q) override
        {
            h(this->simulatorId, GetQubitId(q));
        }

        void ControlledH(long numControls, Qubit controls[], Qubit target) override
        {
            throw std::logic_error("not yet implemented");
        }

        void S(Qubit q) override
        {
            s(this->simulatorId, GetQubitId(q));
        }

        void ControlledS(long numControls, Qubit controls[], Qubit target) override
        {
            throw std::logic_error("not yet implemented");
        }

        void AdjointS(Qubit q) override
        {
            s_adj(this->simulatorId, GetQubitId(q));
        }

        void ControlledAdjointS(long numControls, Qubit controls[], Qubit target) override
        {
            throw std::logic_error("not yet implemented");
        }

        void T(Qubit q) override
        {
            t(this->simulatorId, GetQubitId(q));
        }

        void ControlledT(long numControls, Qubit controls[], Qubit target) override
        {
            throw std::logic_error("not yet implemented");
        }

        void AdjointT(Qubit q) override
        {
            t_adj(this->simulatorId, GetQubitId(q));
        }

        void ControlledAdjointT(long numControls, Qubit controls[], Qubit target) override
        {
            throw std::logic_error("not yet implemented");
        }

        void R(PauliId axis, Qubit target, double theta) override
        {
            throw std::logic_error("not yet implemented");
        }

        void ControlledR(long numControls, Qubit controls[], PauliId axis, Qubit target, double theta) override
        {
            throw std::logic_error("not yet implemented");
        }

        void Exp(long numTargets, PauliId paulis[], Qubit targets[], double theta) override
        {
            throw std::logic_error("not yet implemented");
        }

        void ControlledExp(
            long numControls,
            Qubit controls[],
            long numTargets,
            PauliId paulis[],
            Qubit targets[],
            double theta) override
        {
            throw std::logic_error("not yet implemented");
        }
    };

    std::unique_ptr<IRuntimeDriver> CreateOpenSystemsSimulator()
    {
        return std::make_unique<OpenSystemSimulator>();
    }
} // namespace Quantum
} // namespace Microsoft