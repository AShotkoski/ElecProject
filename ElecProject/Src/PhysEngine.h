#pragma once

#include <DirectXMath.h>
#include <functional>

namespace phys
{
	// Current state of an object
	struct State
	{
		DirectX::XMVECTOR position;
		DirectX::XMVECTOR velocity;
	};
	// Derivative of a state
	struct Derivative
	{
		DirectX::XMVECTOR velocity;
		DirectX::XMVECTOR acceleration;
	};

	// Abstract force class represents different types of forces between objects
	class Force
	{
	public:
		// Pure virtual compute class computes the force vector given a state
		virtual DirectX::XMVECTOR compute(const State& state) = 0;
		virtual ~Force() = default;
	};

	// Gravitational force
	class GravForce : public Force
	{
	public:
		GravForce(const std::vector<State>& affectingObjects, 
			const std::vector<float>& affectingObjMasses, 
			float G, float mass)
			:
			others(affectingObjects)
			, otherMasses(affectingObjMasses)
			, G(G)
			, mass(mass)
		{
		}

		DirectX::XMVECTOR compute(const State& state) override
		{
			using namespace DirectX;
			XMVECTOR netF = XMVectorZero();
			for (size_t i = 0; i < others.size(); ++i)
			{
				const auto& s = others[i];
				float sMass = otherMasses[i];

				XMVECTOR r = XMVectorSubtract(s.position, state.position); // displacement
				XMVECTOR distSqVec = XMVector3Length(r); // This could be sped up
				float distSq;
				XMStoreFloat(&distSq, distSqVec);
				
				// Add a clamp condition to set a minimum distance to avoid ultra high forces
				constexpr const float distSqMin = 2.5e-7;
				if (distSq < distSqMin)
					continue;

				XMVECTOR rHat = XMVector3Normalize(r); // Could be sped up
				float magF = G * mass * sMass / distSq;
				XMVECTOR forceVec = XMVectorScale(rHat, magF);
				netF = XMVectorAdd(netF, forceVec);
			}
			return netF;
		}

	private:
		const std::vector<State>& others; // The other objects that affect this force
		const std::vector<float>& otherMasses; // The indexed list of masses of the other objects
		float G;
		float mass;
	};
	

	// Generic integration function given a state
	// (I could add a time variable here if for some reason I don't
	// have a time-independent system in the future)
	void rk4Integrate(
		State& state, // obj current state
		float dt, // time step
		const std::function<DirectX::XMVECTOR(const State& state)>& accelerationFunction ) // Function to compute acceleration	
	{
		using namespace DirectX;
		// k1 calcs
		Derivative a;
		a.velocity = state.velocity;
		a.acceleration = accelerationFunction(state);

		// k2 calcs
		State state2;
		state2.position = XMVectorAdd(state.position, XMVectorScale(a.velocity, dt * 0.5f));
		state2.velocity = XMVectorAdd(state.velocity, XMVectorScale(a.acceleration, dt * 0.5f));
		Derivative b;
		b.velocity = state2.velocity;
		b.acceleration = accelerationFunction(state2);

		// k3 calcs
		State state3;
		state3.position = XMVectorAdd(state.position, XMVectorScale(b.velocity, dt * 0.5f));
		state3.velocity = XMVectorAdd(state.velocity, XMVectorScale(b.acceleration, dt * 0.5f));
		Derivative c;
		c.velocity = state3.velocity;
		c.acceleration = accelerationFunction(state3);

		// k4 calcs
		State state4;
		state4.position = XMVectorAdd(state.position, XMVectorScale(c.velocity, dt));
		state4.velocity = XMVectorAdd(state.velocity, XMVectorScale(c.acceleration, dt));
		Derivative d;
		d.velocity = state4.velocity;
		d.acceleration = accelerationFunction(state4);

		// Combine derivatives
		XMVECTOR dxdt = XMVectorScale(
			XMVectorAdd(
				XMVectorAdd(a.velocity, XMVectorScale(XMVectorAdd(b.velocity, c.velocity), 2.0f)),
				d.velocity),
			1.0f / 6.0f);

		XMVECTOR dvdt = XMVectorScale(
			XMVectorAdd(
				XMVectorAdd(a.acceleration,XMVectorScale(XMVectorAdd(b.acceleration, c.acceleration), 2.0f)),
				d.acceleration),
			1.0f / 6.0f);

		// Update the obj state
		state.position = XMVectorAdd(state.position, XMVectorScale(dxdt, dt));
		state.velocity = XMVectorAdd(state.velocity, XMVectorScale(dvdt, dt));
	}
	
}