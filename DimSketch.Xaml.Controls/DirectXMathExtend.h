#pragma once

#ifndef DX_MATH_EXT_H
#define DX_MATH_EXT_H

#if defined(BT_USE_SSE)
#error Bullet physics can not be include before DirectX Math headers.
#endif

// Disable bullet to overload operators on _m128
#ifndef BT_NO_SIMD_OPERATOR_OVERLOADS
#define BT_NO_SIMD_OPERATOR_OVERLOADS
#endif


#include <DirectXMath.h>
#include <DirectXCollision.h>
#include <DirectXColors.h>

#include <DirectXMathSSE3.h>
#include <DirectXMathSSE4.h>
#include <DirectXMathAVX.h>
#include <DirectXMathAVX2.h>

#include <SimpleMath.h>

#ifndef XM_ALIGNATTR
#define XM_ALIGNATTR _MM_ALIGN16
#endif
namespace DirectX
{
	const float XM_EPSILON = 1.192092896e-7f;
	const float XM_INFINITY = 1e12f;

	// vector alignment of XMVECTOR
	const size_t XM_ALIGNMENT = alignof(XMVECTOR);

	XMGLOBALCONST XMVECTORF32 g_XMNegtiveXZ = { -1.0f,1.0f,-1.0f,1.0f };

	inline float XMScalarReciprocalSqrtEst(float x) {
		float xhalf = 0.5f*x;
		int i = *(int*)&x;
		i = 0x5f3759df - (i >> 1);
		x = *(float*)&i;
		x = x*(1.5f - xhalf*x*x);
		return x;
	}

	inline XMMATRIX XMMatrixZero()
	{
		XMMATRIX M;
		M.r[0] = XMVectorZero();
		M.r[1] = XMVectorZero();
		M.r[2] = XMVectorZero();
		M.r[3] = XMVectorZero();
		return M;
	}

	inline XMVECTOR XM_CALLCONV XMLoadFloat4(const float* pF4)
	{
		return XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(pF4));
	}
	inline XMVECTOR XM_CALLCONV XMLoadFloat3(const float* pF3)
	{
		return XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(pF3));
	}
	inline void XM_CALLCONV XMStoreFloat4(
		float* pF4,
		FXMVECTOR  V
		)
	{
		return XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(pF4), V);
	}
	inline void XM_CALLCONV XMStoreFloat3(
		float* pF3,
		FXMVECTOR  V
		)
	{
		return XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(pF3), V);
	}

	// return V.x + V.y + V.z + V.w
	inline XMVECTOR XM_CALLCONV XMVector4HorizontalSum(FXMVECTOR V)
	{
#if defined(_XM_NO_INTRINSICS_)

		XMVECTOR Result;
		Result.vector4_f32[0] =
			Result.vector4_f32[1] =
			Result.vector4_f32[2] =
			Result.vector4_f32[3] = V.vector4_f32[0] + V.vector4_f32[1] + V.vector4_f32[2] + V.vector4_f32[3];
		return Result;

#elif defined(_XM_ARM_NEON_INTRINSICS_)
		static_assert(false, "ARM Instrinsics not available yet");
		float32x4_t vTemp = vmulq_f32(V1, V2);
		float32x2_t v1 = vget_low_f32(vTemp);
		float32x2_t v2 = vget_high_f32(vTemp);
		v1 = vpadd_f32(v1, v1);
		v2 = vpadd_f32(v2, v2);
		v1 = vadd_f32(v1, v2);
		return vcombine_f32(v1, v1);
#elif defined(_XM_SSE_INTRINSICS_)
		//static_assert(false, "SIDM Instrinsics not tested yet");
		XMVECTOR vT = V;
		vT = _mm_shuffle_ps(vT, V, _MM_SHUFFLE(1, 0, 0, 0)); // Copy X to the Z position and Y to the W position
		vT = _mm_add_ps(vT, V);          // Add Z = X+Z; W = Y+W;
		vT = _mm_shuffle_ps(V, vT, _MM_SHUFFLE(0, 3, 0, 0));  // Copy W to the Z position
		vT = _mm_add_ps(V, vT);           // Add Z and W together
		return XM_PERMUTE_PS(V, _MM_SHUFFLE(2, 2, 2, 2));    // Splat Z and return
#endif
	}

#ifndef DIRECTX_ALLIGNED_NEW
#define DIRECTX_ALLIGNED_NEW
	template<typename T>
	struct AlignedNew
	{
		static const size_t alignment = alignof(T);
		static_assert(alignment > 8, "AlignedNew is only useful for types with > 8 byte alignment. Did you forget a _declspec(align) on TDerived?");
		// Allocate aligned memory.
		static void* operator new (size_t size)
		{

		void* ptr = _aligned_malloc(size, alignment);

		if (!ptr)
			throw std::bad_alloc();

		return ptr;
		}


			// Free aligned memory.
			static void operator delete (void* ptr)
		{
			_aligned_free(ptr);
		}


		// Array overloads.
		static void* operator new[](size_t size)
		{
			return operator new(size);
		}


			static void operator delete[](void* ptr)
		{
			operator delete(ptr);
		}
	};

	template <typename _T, size_t _Align_Boundary = std::alignment_of<_T>::value>
	class AlignedAllocator
	{
	public:
		typedef	_T		 value_type;
		typedef	size_t		 size_type;
		typedef	ptrdiff_t	 difference_type;
		typedef	_T		*pointer;
		typedef const _T		*const_pointer;
		typedef	_T		&reference;
		typedef const _T		&const_reference;
		inline AlignedAllocator() throw() {}

		template <typename _T2>
		inline  AlignedAllocator(const AlignedAllocator<_T2, _Align_Boundary> &) throw() {}
		inline ~AlignedAllocator() throw() {}

		template <size_t _Other_Alignment>
		inline bool operator== (const AlignedAllocator<_T, _Other_Alignment> & rhs) const
		{
			return _Align_Boundary == _Other_Alignment;
		}



		inline pointer adress(reference r)
		{
			return &r;
		}

		inline const_pointer adress(const_reference r) const
		{
			return &r;
		}

		inline pointer allocate(size_type n)
		{
			return (pointer)_mm_malloc(n*sizeof(value_type), _Align_Boundary);
		}

		inline void deallocate(pointer p, size_type)
		{
			_mm_free(p);
		}

		inline void construct(pointer p, const_reference wert)
		{
			::new(p) value_type(wert);
		}

		inline void destroy(pointer p)
		{ /* C4100 */ p; p->~value_type();
		}

		inline size_type max_size() const throw()
		{
			return size_type(-1) / sizeof(value_type);
		}

		template <typename _T2>
		struct rebind { typedef AlignedAllocator<_T2, _Align_Boundary> other; };
	};
#endif //DIRECTX_ALLIGNED_NEW

#ifdef _MSC_VER
#pragma region XMFLOAT3X4
#endif
	// 3x4 Matrix: 32 bit floating point components
	// Row Major , Usually use to pass float4x3 matrix to GPU
	// Transpose befor store and after load
	struct XMFLOAT3X4
	{
		union
		{
			struct
			{
				float _11, _12, _13, _14;
				float _21, _22, _23, _24;
				float _31, _32, _33, _34;
			};
			float m[3][4];
		};

		XMFLOAT3X4() {}
		XMFLOAT3X4(float m00, float m01, float m02, float m03,
			float m10, float m11, float m12, float m13,
			float m20, float m21, float m22, float m23);
		explicit XMFLOAT3X4(_In_reads_(12) const float *pArray);

		float       operator() (size_t Row, size_t Column) const { return m[Row][Column]; }
		float&      operator() (size_t Row, size_t Column) { return m[Row][Column]; }

		XMFLOAT3X4& operator= (const XMFLOAT3X4& Float4x3);
	};

	inline XMFLOAT3X4::XMFLOAT3X4
		(
			float m00, float m01, float m02, float m03,
			float m10, float m11, float m12, float m13,
			float m20, float m21, float m22, float m23
			)
	{
		m[0][0] = m00;
		m[0][1] = m01;
		m[0][2] = m02;
		m[0][3] = m03;

		m[1][0] = m10;
		m[1][1] = m11;
		m[1][2] = m12;
		m[1][3] = m13;

		m[2][0] = m20;
		m[2][1] = m21;
		m[2][2] = m22;
		m[2][3] = m23;

	}

	//------------------------------------------------------------------------------
	_Use_decl_annotations_
		inline XMFLOAT3X4::XMFLOAT3X4
		(
			const float* pArray
			)
	{
		assert(pArray != nullptr);

		m[0][0] = pArray[0];
		m[0][1] = pArray[1];
		m[0][2] = pArray[2];
		m[0][3] = pArray[3];

		m[1][0] = pArray[4];
		m[1][1] = pArray[5];
		m[1][2] = pArray[6];
		m[1][3] = pArray[7];

		m[2][0] = pArray[8];
		m[2][1] = pArray[9];
		m[2][2] = pArray[10];
		m[2][3] = pArray[11];

	}

	//------------------------------------------------------------------------------
	inline XMFLOAT3X4& XMFLOAT3X4::operator=
		(
			const XMFLOAT3X4& Float4x4
			)
	{
		XMVECTOR V1 = XMLoadFloat4((const XMFLOAT4*)&Float4x4._11);
		XMVECTOR V2 = XMLoadFloat4((const XMFLOAT4*)&Float4x4._21);
		XMVECTOR V3 = XMLoadFloat4((const XMFLOAT4*)&Float4x4._31);

		XMStoreFloat4((XMFLOAT4*)&_11, V1);
		XMStoreFloat4((XMFLOAT4*)&_21, V2);
		XMStoreFloat4((XMFLOAT4*)&_31, V3);

		return *this;
	}

	inline void XMStoreFloat3x4(
		XMFLOAT3X4 *pDestination,
		CXMMATRIX M
		)
	{
		assert(pDestination);
#if defined(_XM_NO_INTRINSICS_)

		pDestination->m[0][0] = M.r[0].vector4_f32[0];
		pDestination->m[0][1] = M.r[0].vector4_f32[1];
		pDestination->m[0][2] = M.r[0].vector4_f32[2];
		pDestination->m[0][3] = M.r[0].vector4_f32[3];

		pDestination->m[1][0] = M.r[1].vector4_f32[0];
		pDestination->m[1][1] = M.r[1].vector4_f32[1];
		pDestination->m[1][2] = M.r[1].vector4_f32[2];
		pDestination->m[1][3] = M.r[1].vector4_f32[3];

		pDestination->m[2][0] = M.r[2].vector4_f32[0];
		pDestination->m[2][1] = M.r[2].vector4_f32[1];
		pDestination->m[2][2] = M.r[2].vector4_f32[2];
		pDestination->m[2][3] = M.r[2].vector4_f32[3];

#elif defined(_XM_ARM_NEON_INTRINSICS_)
		vst1q_f32(reinterpret_cast<float*>(&pDestination->_11), M.r[0]);
		vst1q_f32(reinterpret_cast<float*>(&pDestination->_21), M.r[1]);
		vst1q_f32(reinterpret_cast<float*>(&pDestination->_31), M.r[2]);
#elif defined(_XM_SSE_INTRINSICS_)
		_mm_storeu_ps(&pDestination->_11, M.r[0]);
		_mm_storeu_ps(&pDestination->_21, M.r[1]);
		_mm_storeu_ps(&pDestination->_31, M.r[2]);
#else // _XM_VMX128_INTRINSICS_
#endif // _XM_VMX128_INTRINSICS_
	}

	inline XMMATRIX XMLoadFloat3x4
		(
			const XMFLOAT3X4* pSource
			)
	{
		assert(pSource);
#if defined(_XM_NO_INTRINSICS_)

		XMMATRIX M;
		M.r[0].vector4_f32[0] = pSource->m[0][0];
		M.r[0].vector4_f32[1] = pSource->m[0][1];
		M.r[0].vector4_f32[2] = pSource->m[0][2];
		M.r[0].vector4_f32[3] = pSource->m[0][3];

		M.r[1].vector4_f32[0] = pSource->m[1][0];
		M.r[1].vector4_f32[1] = pSource->m[1][1];
		M.r[1].vector4_f32[2] = pSource->m[1][2];
		M.r[1].vector4_f32[3] = pSource->m[1][3];

		M.r[2].vector4_f32[0] = pSource->m[2][0];
		M.r[2].vector4_f32[1] = pSource->m[2][1];
		M.r[2].vector4_f32[2] = pSource->m[2][2];
		M.r[2].vector4_f32[3] = pSource->m[2][3];

		return M;

#elif defined(_XM_ARM_NEON_INTRINSICS_)
		XMMATRIX M;
		M.r[0] = vld1q_f32(reinterpret_cast<const float*>(&pSource->_11));
		M.r[1] = vld1q_f32(reinterpret_cast<const float*>(&pSource->_21));
		M.r[2] = vld1q_f32(reinterpret_cast<const float*>(&pSource->_31));
		return M;
#elif defined(_XM_SSE_INTRINSICS_)
		XMMATRIX M;
		M.r[0] = _mm_loadu_ps(&pSource->_11);
		M.r[1] = _mm_loadu_ps(&pSource->_21);
		M.r[2] = _mm_loadu_ps(&pSource->_31);
		M.r[3] = g_XMIdentityR3;
		return M;
#elif defined(XM_NO_MISALIGNED_VECTOR_ACCESS)
#endif // _XM_VMX128_INTRINSICS_
	}

#ifdef _MSC_VER
#pragma endregion

#pragma region XMDUALVECTOR
#endif

	struct XMDUALVECTOR;
	// Calling convetion

	// First XMDUALVECTOR
#if ( defined(_M_IX86) || defined(_M_ARM) || defined(_XM_VMX128_INTRINSICS_) || _XM_VECTORCALL_ ) && !defined(_XM_NO_INTRINSICS_)
	typedef const XMDUALVECTOR FXMDUALVECTOR;
#else
	typedef const XMDUALVECTOR& FXMDUALVECTOR;
#endif

	// 2nd
#if ( defined(_M_ARM) || defined(_XM_VMX128_INTRINSICS_) || (_XM_VECTORCALL_)) && !defined(_XM_NO_INTRINSICS_)
	typedef const XMDUALVECTOR GXMDUALVECTOR;
#else
	typedef const XMDUALVECTOR& GXMDUALVECTOR;
#endif
	// 3rd
#if ( defined(_XM_VMX128_INTRINSICS_) || _XM_VECTORCALL_ ) && !defined(_XM_NO_INTRINSICS_)
	typedef const XMDUALVECTOR HXMDUALVECTOR;
#else
	typedef const XMDUALVECTOR& HXMDUALVECTOR;
#endif

#if defined(_XM_VMX128_INTRINSICS_) && !defined(_XM_NO_INTRINSICS_)
	typedef const XMDUALVECTOR CXMDUALVECTOR;
#else
	typedef const XMDUALVECTOR& CXMDUALVECTOR;
#endif

#if (defined(_M_IX86) || defined(_M_AMD64) || defined(_M_ARM)) && defined(_XM_NO_INTRINSICS_)
	struct XMDUALVECTOR
#else
	__declspec(align(16)) struct XMDUALVECTOR
#endif
	{
#ifdef _XM_NO_INTRINSICS_
		union
		{
			XMVECTOR r[2];
			struct
			{
				float _11, _12, _13, _14;
				float _21, _22, _23, _24;
			};
			float m[2][4];
		};
#else
		XMVECTOR r[2];
#endif

		XMDUALVECTOR() {}
		XMDUALVECTOR(CXMVECTOR V0, CXMVECTOR V1) { r[0] = V0; r[1] = V1; }
		XMDUALVECTOR(float m00, float m01, float m02, float m03,
			float m10, float m11, float m12, float m13)
		{
			r[0] = XMVectorSet(m00, m01, m02, m03);
			r[1] = XMVectorSet(m10, m11, m12, m13);
		}
		explicit XMDUALVECTOR(_In_reads_(8) const float *pArray)
		{
			r[0] = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(pArray));
			r[1] = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(pArray + 4));
		}

		XMDUALVECTOR& XM_CALLCONV operator= (FXMDUALVECTOR DV) { r[0] = DV.r[0]; r[1] = DV.r[1]; return *this; }

		XMDUALVECTOR XM_CALLCONV operator+ () const { return *this; }
		XMDUALVECTOR XM_CALLCONV operator- () const
		{
			XMDUALVECTOR dvResult(r[0], -r[1]);
			return dvResult;
		}


		XMDUALVECTOR& XM_CALLCONV operator+= (FXMDUALVECTOR M)
		{
			r[0] += M.r[0];
			r[1] += M.r[1];
			return *this;
		}

		XMDUALVECTOR& XM_CALLCONV operator-= (FXMDUALVECTOR M)
		{
			r[0] -= M.r[0];
			r[1] -= M.r[1];
			return *this;
		}

		XMDUALVECTOR&   operator*= (float S)
		{
			r[0] *= S;
			r[1] *= S;
			return *this;
		}

		XMDUALVECTOR&   operator/= (float S)
		{
			r[0] /= S;
			r[1] /= S;
			return *this;
		}



		//friend XMDUALVECTOR operator* (float S, CXMMATRIX M);
	};

#if defined(_XM_SSE_INTRINSICS_) 
	template <>
	inline XMVECTOR XM_CALLCONV XMVectorSwizzle<1, 1, 3, 3>(FXMVECTOR v)
	{
		return SSE3::XMVectorSwizzle_1133(v);
	}
	template <>
	inline XMVECTOR XM_CALLCONV XMVectorSwizzle<0, 0, 2, 2>(FXMVECTOR v)
	{
		return SSE3::XMVectorSwizzle_0022(v);
	}
	template <>
	inline XMVECTOR XM_CALLCONV XMVectorSwizzle<0, 0, 1, 1>(FXMVECTOR v)
	{
		return XMVectorMergeXY(v, v);
	}
	template <>
	inline XMVECTOR XM_CALLCONV XMVectorSwizzle<2, 2, 3, 3>(FXMVECTOR v)
	{
		return XMVectorMergeZW(v, v);
	}
#endif

	// Caculate the rotation quaternion base on v1 and v2 (shortest rotation geo-distance in sphere surface)
	inline XMVECTOR XM_CALLCONV XMQuaternionRotationVectorToVector(FXMVECTOR v1, FXMVECTOR v2) {
		assert(!XMVector3Equal(v1, XMVectorZero()));
		assert(!XMVector3Equal(v2, XMVectorZero()));
		XMVECTOR n1 = XMVector3Normalize(v1);
		XMVECTOR n2 = XMVector3Normalize(v2);
		XMVECTOR axias = XMVector3Cross(n1, n2);
		if (XMVector4NearEqual(axias, g_XMZero.v, g_XMEpsilon.v))
		{
			n2 = g_XMIdentityR0.v;
			axias = XMVector3Cross(n1, n2);
			if (XMVector4NearEqual(axias, g_XMZero.v, g_XMEpsilon.v))
			{
				n2 = g_XMIdentityR1.v;
				axias = XMVector3Cross(n1, n2);
			}
		}
		float angle = std::acosf(XMVectorGetX(XMVector3Dot(n1, n2)));
		auto rot = XMQuaternionRotationAxis(axias, angle);
		return rot;
	}

	enum AxisEnum
	{
		AxisX = 0,
		AxisY = 1,
		AxisZ = 2,
	};

	template <unsigned X, unsigned Y, unsigned Z>
	// Caculate Left handed eular angles with 
	// rotation sequence R(X)-R(Y)-R(Z)
	XMVECTOR XM_CALLCONV XMQuaternionEulerAngleABC(FXMVECTOR q);

	template <>
	// Caculate Left handed eular angles with 
	// rotation sequence R(Z)-R(Y)-R(X)
	inline XMVECTOR XM_CALLCONV XMQuaternionEulerAngleABC<2, 1, 0>(FXMVECTOR q)
	{
#if defined(_XM_NO_INTRINSICS_)
		float q0 = q.vector4_f32[3], q1 = q.vector4_f32[1], q2 = q.vector4_f32[2], q3 = q.vector4_f32[3];
		XMVECTOR euler;
		euler.vector4_f32[0] = atan2(2 * (q0*q1 + q2*q3), 1 - 2 * (q1*q1 + q2*q2));
		euler.vector4_f32[1] = asinf(2 * (q0*q2 - q3*q1));
		euler.vector4_f32[2] = atan2(2 * (q0*q3 + q2*q1), 1 - 2 * (q2*q2 + q3*q3));
		return euler;
#elif defined(_XM_SSE_INTRINSICS_) || defined(_XM_ARM_NEON_INTRINSICS_)

		XMVECTOR q20 = XMVectorSwizzle<1, 1, 3, 3>(q);
		XMVECTOR q13 = XMVectorSwizzle<0, 0, 2, 2>(q);

		// eular X - Roll
		XMVECTOR X = XMVector4Dot(q20, q13);
		XMVECTOR Y = XMVectorMergeXY(q13, q20);
		Y = XMVector4Dot(Y, Y);
		Y = XMVectorSubtract(g_XMOne.v, Y);
		XMVECTOR vResult = XMVectorATan2(Y, X);

		// eular Z - X
		q13 = XMVectorSwizzle<2, 2, 0, 0>(q); // now 3311
		X = XMVector4Dot(q20, q13);
		Y = XMVectorMergeXY(q13, q20);
		Y = XMVector4Dot(Y, Y);
		Y = XMVectorSubtract(g_XMOne.v, Y);
		Y = XMVectorATan2(Y, X);

		// eular Y - Patch
		X = XMVectorSwizzle<2, 3, 0, 1>(q);
		X = XMVectorMultiply(X, g_XMNegtiveXZ.v);
		X = XMVector4Dot(X, q);

		// merge result
		Y = XMVectorMergeXY(Y, X);
		vResult = XMVectorSelect(Y, vResult, g_XMSelect1000.v);
		vResult = XMVectorAndInt(vResult, g_XMMask3.v);
		return vResult;
#endif
	}

	XMVECTOR XM_CALLCONV XMVector3Displacement(FXMVECTOR V, FXMVECTOR RotationQuaternion, FXMVECTOR TranslationQuaternion);

	inline XMDUALVECTOR XM_CALLCONV XMDualQuaternionTranslation(FXMVECTOR T)
	{
		static const XMVECTORF32 Control = { 0.5f,0.5f,0.5f,.0f };
		XMVECTOR Qe = XMVectorMultiply(T, Control);
		XMVECTOR Qr = XMQuaternionIdentity();
		XMDUALVECTOR dqRes;
		dqRes.r[0] = Qr;
		dqRes.r[1] = Qe;
		return dqRes;
	}

	inline XMDUALVECTOR XM_CALLCONV XMDualQuaternionRotation(FXMVECTOR Q)
	{
		XMDUALVECTOR dqRes;
		dqRes.r[0] = Q;
		dqRes.r[1] = XMVectorZero();
	}

	inline XMDUALVECTOR XM_CALLCONV XMDualQuaternionRigidTransform(FXMVECTOR RotationOrigin, FXMVECTOR RotationQuaternion, FXMVECTOR Translation)
	{
		static const XMVECTORF32 Control = { 0.5f,0.5f,0.5f,.0f };

		XMVECTOR Qr = RotationQuaternion;
		XMVECTOR Qe = RotationOrigin + Translation;
		Qe = XMVectorMultiply(Qe, Control);
		XMVECTOR Qo = XMVectorMultiply(RotationOrigin, Control);
		Qo = XMVectorNegate(Qo);

		Qe = XMQuaternionMultiply(Qr, Qe);
		Qo = XMQuaternionMultiply(Qo, Qr);
		Qe += Qo;

		XMDUALVECTOR dqRes;
		dqRes.r[0] = Qr;
		dqRes.r[1] = Qe;
		return dqRes;
	}

	inline XMDUALVECTOR XM_CALLCONV XMDualQuaternionRotationTranslation(FXMVECTOR Q, FXMVECTOR T)
	{
		// non-dual part (just copy q0):
		// dual part:
		static const XMVECTORF32 Control = { 0.5f,0.5f,0.5f,.0f };
		XMVECTOR Qe = XMVectorMultiply(T, Control);
		Qe = XMQuaternionMultiply(Q, Qe);
		XMDUALVECTOR dqRes;
		dqRes.r[0] = Q;
		dqRes.r[1] = Qe;

		return dqRes;
	};

	inline XMDUALVECTOR XM_CALLCONV XMDualQuaternionNormalize(FXMDUALVECTOR Dq)
	{
		XMVECTOR Length = XMQuaternionLength(Dq.r[0]);
		XMDUALVECTOR dqRes;
		dqRes.r[0] = XMVectorDivide(Dq.r[0], Length);
		dqRes.r[1] = XMVectorDivide(Dq.r[1], Length);
		return dqRes;
	}
	inline XMVECTOR XM_CALLCONV XMDualQuaternionNormalizeEst(FXMDUALVECTOR Dq)
	{
		return XMVector4NormalizeEst(Dq.r[0]);
	}
	inline XMVECTOR XM_CALLCONV XMDualQuaternionLength(FXMDUALVECTOR Dq)
	{
		return XMVector4Length(Dq.r[0]);
	}
	inline XMVECTOR XM_CALLCONV XMDualQuaternionLengthSq(FXMDUALVECTOR Dq)
	{
		return XMVector4LengthSq(Dq.r[0]);
	}
	inline XMVECTOR XM_CALLCONV XMDualQuaternionLengthEst(FXMDUALVECTOR Dq)
	{
		return XMVector4LengthEst(Dq.r[0]);
	}
	inline XMDUALVECTOR XM_CALLCONV XMDualVectorConjugate(FXMDUALVECTOR Dq)
	{
		XMDUALVECTOR dvResult(Dq.r[0], XMVectorNegate(Dq.r[1]));
		return dvResult;
	}
	inline XMDUALVECTOR XM_CALLCONV XMDualQuaternionConjugate(FXMDUALVECTOR Dq)
	{
		XMDUALVECTOR dvResult;
		dvResult.r[0] = XMQuaternionConjugate(Dq.r[0]);
		dvResult.r[1] = XMQuaternionConjugate(Dq.r[1]);
		return dvResult;
	}

	inline XMDUALVECTOR XM_CALLCONV XMDualQuaternionInverse(FXMDUALVECTOR Dq)
	{
		XMDUALVECTOR dvResult = XMDualQuaternionConjugate(Dq);
		dvResult.r[0] = XMQuaternionNormalize(dvResult.r[0]);
	}

	inline XMDUALVECTOR XM_CALLCONV XMDualQuaternionMultipy(FXMDUALVECTOR DQ0, GXMDUALVECTOR DQ1)
	{
		XMDUALVECTOR dvResult;
		dvResult.r[0] = XMQuaternionMultiply(DQ0.r[0], DQ1.r[1]);
		dvResult.r[1] = XMQuaternionMultiply(DQ0.r[0], DQ1.r[1]);
		dvResult.r[1] += XMQuaternionMultiply(DQ0.r[1], DQ1.r[0]);
		return dvResult;
	}

	inline bool XM_CALLCONV XMDualQuaternionDecompose(XMVECTOR* outRotQuat, XMVECTOR* outTrans, FXMDUALVECTOR Dq)
	{
		static const XMVECTORF32 ControlX = { 2.0f,-2.0f,2.0f,-2.0f };
		static const XMVECTORF32 ControlY = { 2.0f,2.0f,-2.0f,-2.0f };
		static const XMVECTORF32 ControlZ = { -2.0f,2.0f,2.0f,-2.0f };

		//vT.x = 2.0f*( Qe.x*Q.w - Qe.y*Q.z + Qe.z*Q.y - Qe.w*Q.x);
		//vT.y = 2.0f*( Qe.x*Q.z + Qe.y*Q.w - Qe.z*Q.x - Qe.w*Q.y);
		//vT.z = 2.0f*(-Qe.x*Q.y + Qe.y*Q.x + Qe.z*Q.w - Qe.w*Q.z);
		XMVECTOR vT;
		XMVECTOR Qr = Dq.r[0];
		XMVECTOR Qe = Dq.r[1];
		Qr = XMQuaternionNormalize(Qr);

		XMVECTOR Q = XMVectorSwizzle<3, 2, 1, 0>(Qr);
		Q = XMVectorMultiply(Qe, Q);
		Q = XMVector4Dot(Q, ControlX.v);
		vT = XMVectorAndInt(Q, g_XMMaskX.v);

		Q = XMVectorSwizzle<2, 3, 0, 1>(Qr);
		Q = XMVectorMultiply(Qe, Q);
		Q = XMVector4Dot(Q, ControlY.v);
		Q = XMVectorAndInt(Q, g_XMMaskY.v);
		vT = XMVectorOrInt(vT, Q);

		Q = XMVectorSwizzle<1, 0, 3, 2>(Qr);
		Q = XMVectorMultiply(Qe, Q);
		Q = XMVector4Dot(Q, ControlZ.v);
		Q = XMVectorAndInt(Q, g_XMMaskZ.v);
		vT = XMVectorOrInt(vT, Q);

		*outRotQuat = Qr;
		*outTrans = vT;
		return true;
	}

	inline XMVECTOR XM_CALLCONV XMVector3Displacement(FXMVECTOR V, FXMVECTOR RotationQuaternion, FXMVECTOR TranslationQuaternion)
	{
		XMVECTOR Qr = RotationQuaternion;
		XMVECTOR Qe = TranslationQuaternion;
		XMVECTOR Dual = XMVector4Dot(Qr, Qe);
		assert(XMVector4NearEqual(Dual, XMVectorZero(), XMVectorReplicate(0.01f)));

		XMVECTOR vRes = XMVector3Rotate(V, Qr);
		Qr = XMQuaternionConjugate(Qr);
		XMVECTOR vTrans = XMQuaternionMultiply(Qr, Qe);
		//XMVECTOR vTrans = XMVectorSplatW(Qr) * Qe - Qr * XMVectorSplatW(Qe) + XMVector3Cross(Qr,Qe);
		vTrans *= g_XMTwo.v;

		vRes += vTrans;
		return vRes;
	}


	inline XMVECTOR CaculatePrincipleAxisFromPoints(size_t Count, const XMFLOAT3 * pPoints, size_t Stride, float pointsUpperBound)
	{
		XMVECTOR InvScale = XMVectorReplicate(1.0f / pointsUpperBound);
		XMVECTOR CenterOfMass = XMVectorZero();

		// Compute the center of mass and inertia tensor of the points.
		for (size_t i = 0; i < Count; ++i)
		{
			XMVECTOR Point = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(reinterpret_cast<const uint8_t*>(pPoints) + i * Stride));

			CenterOfMass += Point * InvScale; // Only Modify is here, normalize scales to 1 to prevent float overflow
		}

		CenterOfMass *= XMVectorReciprocal(XMVectorReplicate(float(Count)));
		XMVECTOR NegCenterOfMass = -CenterOfMass;

		// Compute the inertia tensor of the points around the center of mass.
		// Using the center of mass is not strictly necessary, but will hopefully
		// improve the stability of finding the eigenvectors.
		XMVECTOR XX_YY_ZZ = XMVectorZero();
		XMVECTOR XY_XZ_YZ = XMVectorZero();

		for (size_t i = 0; i < Count; ++i)
		{
			XMVECTOR Point = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(reinterpret_cast<const uint8_t*>(pPoints) + i * Stride));
			Point = XMVectorMultiplyAdd(Point, InvScale, NegCenterOfMass); // normalize scales to 1 to prevent float overflow

			XX_YY_ZZ += Point * Point;

			XMVECTOR XXY = XMVectorSwizzle<XM_SWIZZLE_X, XM_SWIZZLE_X, XM_SWIZZLE_Y, XM_SWIZZLE_W>(Point);
			XMVECTOR YZZ = XMVectorSwizzle<XM_SWIZZLE_Y, XM_SWIZZLE_Z, XM_SWIZZLE_Z, XM_SWIZZLE_W>(Point);

			XY_XZ_YZ += XXY * YZZ;
		}

		XMVECTOR v1, v2, v3;

		// Compute the eigenvectors of the inertia tensor.
		DirectX::Internal::CalculateEigenVectorsFromCovarianceMatrix(XMVectorGetX(XX_YY_ZZ), XMVectorGetY(XX_YY_ZZ),
			XMVectorGetZ(XX_YY_ZZ),
			XMVectorGetX(XY_XZ_YZ), XMVectorGetY(XY_XZ_YZ),
			XMVectorGetZ(XY_XZ_YZ),
			&v1, &v2, &v3);

		// Put them in a matrix.
		XMMATRIX R;

		R.r[0] = XMVectorSetW(v1, 0.f);
		R.r[1] = XMVectorSetW(v2, 0.f);
		R.r[2] = XMVectorSetW(v3, 0.f);
		R.r[3] = g_XMIdentityR3.v;

		// Multiply by -1 to convert the matrix into a right handed coordinate 
		// system (Det ~= 1) in case the eigenvectors form a left handed 
		// coordinate system (Det ~= -1) because XMQuaternionRotationMatrix only 
		// works on right handed matrices.
		XMVECTOR Det = XMMatrixDeterminant(R);

		if (XMVector4Less(Det, XMVectorZero()))
		{
			R.r[0] *= g_XMNegativeOne.v;
			R.r[1] *= g_XMNegativeOne.v;
			R.r[2] *= g_XMNegativeOne.v;
		}

		// Get the rotation quaternion from the matrix.
		XMVECTOR vOrientation = XMQuaternionRotationMatrix(R);

		// Make sure it is normal (in case the vectors are slightly non-orthogonal).
		vOrientation = XMQuaternionNormalize(vOrientation);

		return vOrientation;
		//// Rebuild the rotation matrix from the quaternion.
		//R = XMMatrixRotationQuaternion(vOrientation);

		//// Build the rotation into the rotated space.
		//XMMATRIX InverseR = XMMatrixTranspose(R);
	}

	// Create AABB & OBB from points, also normalize points extents to prevent float overflow during caculating principle axis
	inline void CreateBoundingBoxesFromPoints(BoundingBox & aabb, BoundingOrientedBox & obb, size_t Count, const XMFLOAT3 * pPoints, size_t Stride) {
		BoundingBox::CreateFromPoints(aabb, Count, pPoints, Stride);
		float pointsUpperBound = XMMax(XMMax(aabb.Extents.x, aabb.Extents.y), aabb.Extents.z);


		XMVECTOR vOrientation = CaculatePrincipleAxisFromPoints(Count, pPoints, Stride, pointsUpperBound);

		// Rebuild the rotation matrix from the quaternion.
		XMMATRIX R = XMMatrixRotationQuaternion(vOrientation);

		// Build the rotation into the rotated space.
		XMMATRIX InverseR = XMMatrixTranspose(R);

		// Find the minimum OBB using the eigenvectors as the axes.
		XMVECTOR vMin, vMax;

		vMin = vMax = XMVector3TransformNormal(XMLoadFloat3(pPoints), InverseR);

		for (size_t i = 1; i < Count; ++i)
		{
			XMVECTOR Point = XMVector3TransformNormal(XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(reinterpret_cast<const uint8_t*>(pPoints) + i * Stride)),
				InverseR);

			vMin = XMVectorMin(vMin, Point);
			vMax = XMVectorMax(vMax, Point);
		}

		// Rotate the center into world space.
		XMVECTOR vCenter = (vMin + vMax) * 0.5f;
		vCenter = XMVector3TransformNormal(vCenter, R);

		// Store center, extents, and orientation.
		XMStoreFloat3(&obb.Center, vCenter);
		XMStoreFloat3(&obb.Extents, (vMax - vMin) * 0.5f);
		XMStoreFloat4(&obb.Orientation, vOrientation);
	}

	inline XMMATRIX XM_CALLCONV XMMatrixScalingFromCenter(FXMVECTOR scale, FXMVECTOR center)
	{
		XMMATRIX m = XMMatrixScalingFromVector(scale);
		m.r[3] = XMVectorSubtract(g_XMOne.v, scale);
		m.r[3] *= center;
		m.r[3] = XMVectorSelect(g_XMOne.v, m.r[3], g_XMSelect1110.v);
		return m;
	}

	// No Matrix Multiply inside, significant faster than affine transform
	inline XMMATRIX XM_CALLCONV XMMatrixRigidTransform(FXMVECTOR RotationOrigin, FXMVECTOR RotationQuaterion, FXMVECTOR Translation)
	{
		XMVECTOR VTranslation = XMVector3Rotate(RotationOrigin, RotationQuaterion);
		VTranslation = XMVectorAdd(VTranslation, RotationOrigin);
		VTranslation = XMVectorAdd(VTranslation, VTranslation);
		VTranslation = XMVectorSelect(g_XMSelect1110.v, Translation, g_XMSelect1110.v);
		XMMATRIX M = XMMatrixRotationQuaternion(RotationQuaterion);
		M.r[3] = XMVectorAdd(M.r[3], VTranslation);
		return M;
	}

	// No Matrix Multiply inside, significant faster than affine transform
	inline XMMATRIX XM_CALLCONV XMMatrixRigidTransform(FXMVECTOR RotationQuaterion, FXMVECTOR Translation)
	{
		XMMATRIX M = XMMatrixRotationQuaternion(RotationQuaterion);
		XMVECTOR VTranslation = XMVectorSelect(g_XMSelect1110.v, Translation, g_XMSelect1110.v);
		M.r[3] = XMVectorAdd(M.r[3], VTranslation);
		return M;
	}

	inline XMVECTOR XM_CALLCONV XMVector3Displacement(FXMVECTOR V, CXMDUALVECTOR TransformDualQuaternion)
	{
		return XMVector3Displacement(V, TransformDualQuaternion.r[0], TransformDualQuaternion.r[1]);
	}

#ifdef _MSC_VER
#pragma endregion
#endif

	// SimpleMath::DualQuaternion
	namespace SimpleMath
	{
		class DualQuaternion
		{
		public:
			Quaternion Qr, Qe;
			DualQuaternion()
				: Qr(), Qe(0.0f, 0.0f, 0.0f, 0.0f)
			{}
			DualQuaternion(const Quaternion& Rotation, const Vector3& Translation)
				: Qr(Rotation)
			{
				Qr.Normalize();
				const float* Q0 = reinterpret_cast<float*>(&Qr);
				const float* T = reinterpret_cast<const float*>(&Translation);
				Qe.w = -0.5f*(T[0] * Q0[0] + T[1] * Q0[1] + T[2] * Q0[2]);
				Qe.x = 0.5f*(T[0] * Q0[3] + T[1] * Q0[2] - T[2] * Q0[1]);
				Qe.y = 0.5f*(-T[0] * Q0[2] + T[1] * Q0[3] + T[2] * Q0[0]);
				Qe.z = 0.5f*(T[0] * Q0[1] - T[1] * Q0[0] + T[2] * Q0[3]);
			}
			DualQuaternion(FXMVECTOR Qr, FXMVECTOR Qe)
				: Qr(Qr), Qe(Qe)
			{}
			DualQuaternion(CXMDUALVECTOR DQ)
				: Qr(DQ.r[0]), Qe(DQ.r[1])
			{}

			explicit DualQuaternion(_In_reads_(8) const float* pArray)
				: Qr(pArray), Qe(pArray + 4)
			{}
			explicit DualQuaternion(_In_reads_(2) const Quaternion* pQArray)
				: Qr(*pQArray), Qe(*(pQArray + 1))
			{}

			inline operator XMDUALVECTOR() const
			{
				XMDUALVECTOR dqRes;
				dqRes.r[0] = Qr;
				dqRes.r[1] = Qe;
				return dqRes;
			}


			void Normarlize(DualQuaternion& result) const
			{
				XMDUALVECTOR dq = *this;
				dq = XMDualQuaternionNormalize(dq);
				result.Qr = dq.r[0];
				result.Qe = dq.r[1];
			}

			void Normarlize()
			{
				Normarlize(*this);
			}

			void Inverse(DualQuaternion& result) const
			{
				XMDUALVECTOR dq = *this;
				dq = XMDualQuaternionInverse(dq);
				result.Qr = dq.r[0];
				result.Qe = dq.r[1];
			}
			void Inverse()
			{
				Inverse(*this);
			}

			void Conjugate()
			{
				Qr.Conjugate();
				Qe.Conjugate();
			}

			void Conjugate(DualQuaternion& result) const
			{
				result.Qr = XMQuaternionConjugate(Qr);
				result.Qe = XMQuaternionConjugate(Qe);
			}

			Vector2 Norm() const
			{
				Vector2 value;
				XMVECTOR q0 = Qr;
				XMVECTOR q1 = Qe;
				XMVECTOR len = XMQuaternionLength(q0);
				q1 = XMVector4Dot(q0, q1);
				q0 = XMVectorDivide(q1, len);
				q1 = XMVectorSelect(len, q0, g_XMSelect0101);
				value = q1;
				return value;
			}

			bool IsUnit() const
			{
				XMVECTOR q0 = Qr;
				XMVECTOR q1 = Qe;
				q1 = XMVector4Dot(q0, q1);
				return XMVector4NearEqual(q0, g_XMZero.v, g_XMEpsilon.v);
			}

			bool Decompose(Quaternion& Rotation, Vector3& Translation) const
			{
				const auto& Q = Rotation = XMQuaternionNormalize(Qr);
				// translation vector:
				Translation.x = 2.0f*(-Qe.w*Q.x + Qe.x*Q.w - Qe.y*Q.z + Qe.z*Q.y);
				Translation.y = 2.0f*(-Qe.w*Q.y + Qe.x*Q.z + Qe.y*Q.w - Qe.z*Q.x);
				Translation.z = 2.0f*(-Qe.w*Q.z - Qe.x*Q.y + Qe.y*Q.x + Qe.z*Q.w);
			}

			DualQuaternion& operator+= (const DualQuaternion& rhs)
			{
				Qr += rhs.Qr;
				Qe += rhs.Qe;
				return *this;
			}

			DualQuaternion& operator-= (const DualQuaternion& rhs)
			{
				Qr -= rhs.Qr;
				Qe -= rhs.Qe;
				return *this;
			}

			DualQuaternion& operator*= (const DualQuaternion& rhs)
			{
				XMVECTOR A = this->Qr;
				XMVECTOR B = this->Qe;
				XMVECTOR C = rhs.Qr;
				XMVECTOR D = rhs.Qe;
				D = XMQuaternionMultiply(A, D);
				B = XMQuaternionMultiply(B, C);
				Qe = XMVectorAdd(D, B);
				Qr = XMQuaternionMultiply(A, C);
			}

			DualQuaternion& operator*= (float scale)
			{
				Qr *= scale;
				Qr *= scale;
				return *this;
			}

			DualQuaternion& operator/= (float scale)
			{
				float s = 1.0f / scale;
				return (*this) *= s;
			}
		};

		inline DualQuaternion operator+ (const DualQuaternion& lhs, const DualQuaternion& rhs)
		{
			DualQuaternion result = lhs;
			result += rhs;
			return result;
		}
		inline DualQuaternion operator- (const DualQuaternion& lhs, const DualQuaternion& rhs)
		{
			DualQuaternion result = lhs;
			result -= rhs;
			return result;
		}
		inline DualQuaternion operator* (const DualQuaternion& lhs, const DualQuaternion& rhs)
		{
			DualQuaternion result = lhs;
			result *= rhs;
			return result;
		}
		inline DualQuaternion operator* (const DualQuaternion& lhs, float rhs)
		{
			DualQuaternion result = lhs;
			result *= rhs;
			return result;
		}
		inline DualQuaternion operator/ (const DualQuaternion& lhs, float rhs)
		{
			DualQuaternion result = lhs;
			result /= rhs;
			return result;
		}
	}

	using SimpleMath::Vector2;
	using SimpleMath::Vector3;
	using SimpleMath::Vector4;

	using SimpleMath::Quaternion;
	using SimpleMath::DualQuaternion;

	using SimpleMath::Color;
	using SimpleMath::Plane;
	using SimpleMath::Ray;

	typedef SimpleMath::Matrix Matrix4x4;

	using SimpleMath::operator*;
	using SimpleMath::operator+;
	using SimpleMath::operator-;
	using SimpleMath::operator/;

	namespace HLSLVectors
	{
		using float4 = Vector4;
		using float3 = Vector3;
		using float2 = Vector2;
		using uint = uint32_t;
		using uint4 = XMUINT4;
		using uint3 = XMUINT3;
		using uint2 = XMUINT2;
		using int2 = XMINT2;
		using int3 = XMINT3;
		using int4 = XMINT4;
		using float4x4 = Matrix4x4;
		using float4x3 = XMFLOAT3X4;
		using float3x3 = XMFLOAT3X3;
	}

#ifdef _MSC_VER
#pragma region Quaternion Eular Angle Convertions
#endif

	namespace Internal
	{
		template <unsigned _X, unsigned _Y, unsigned _Z>
		struct SwizzleInverse
		{
		};
		template <>
		struct SwizzleInverse<0, 1, 2>
		{
			static const unsigned X = 0;
			static const unsigned Y = 1;
			static const unsigned Z = 2;
		};
		template <>
		struct SwizzleInverse<1, 2, 0>
		{
			static const unsigned X = 2;
			static const unsigned Y = 0;
			static const unsigned Z = 1;
		};
		template <>
		struct SwizzleInverse<2, 1, 0>
		{
			static const unsigned X = 2;
			static const unsigned Y = 1;
			static const unsigned Z = 0;
		};
		template <>
		struct SwizzleInverse<1, 0, 2>
		{
			static const unsigned X = 1;
			static const unsigned Y = 0;
			static const unsigned Z = 2;
		};
		template <>
		struct SwizzleInverse<0, 2, 1>
		{
			static const unsigned X = 0;
			static const unsigned Y = 2;
			static const unsigned Z = 1;
		};
		template <>
		struct SwizzleInverse<2, 0, 1>
		{
			static const unsigned X = 1;
			static const unsigned Y = 2;
			static const unsigned Z = 0;
		};
	}

	template <unsigned X, unsigned Y, unsigned Z >
	// Caculate Left handed eular angles with 3 different rotation axis
	// rotation sequence R(X)-R(Y)-R(Z)	
	inline XMVECTOR XM_CALLCONV XMQuaternionEulerAngleABC(FXMVECTOR q)
	{
		static_assert(X <= 2 && Y <= 2 && Z <= 2 && X != Y && Z != Y && X != Z, "X-Y-Z must be different Axis");

		XMVECTOR qs = XMVectorSwizzle<Z, Y, X, 3>(q);
		qs = XMQuaternionEulerAngleABC<2, 1, 0>(qs);
		using InvS = Internal::SwizzleInverse<X, Y, Z>;
		qs = XMVectorSwizzle<InvS::X, InvS::Y, InvS::Z, 3>(qs);
		return qs;
	}

	template <unsigned X, unsigned Y>
	// Caculate Left handed eular angles with 2 different rotation axis
	// rotation sequence R(X)-R(Y)-R(X)	
	inline XMVECTOR XM_CALLCONV XMQuaternionEulerAngleABA(FXMVECTOR q)
	{
	}

	// Left handed eular angles
	// rotation sequence R(Y)-R(X)-R(Z), so called Yaw-Pitch-Roll
	// return : <R(X),R(Y),R(Z),0>
	inline XMVECTOR XM_CALLCONV XMQuaternionEulerAngleYXZ(FXMVECTOR q)
	{
		return XMQuaternionEulerAngleABC<AxisY, AxisX, AxisZ>(q);
	}

	// left handed eular angles : <Pitch,Yaw,Roll,0>, ready with XMQuaternionRotationYawPitchRoll
	// rotation sequence R(Y)-R(X)-R(Z), so called Yaw-Pitch-Roll
	inline XMVECTOR XM_CALLCONV XMQuaternionEulerAngleYawPitchRoll(FXMVECTOR q)
	{
		return XMQuaternionEulerAngleABC<AxisY, AxisX, AxisZ>(q);
	}

#ifdef _MSC_VER
#pragma endregion
#endif

	// This function garuntee the extends of the bounding box is sorted from bigger to smaller
	inline void CreateBoundingOrientedBoxFromPoints(_Out_ BoundingOrientedBox& Out, _In_ size_t Count,
		_In_reads_bytes_(sizeof(XMFLOAT3) + Stride*(Count - 1)) const XMFLOAT3* pPoints, _In_ size_t Stride) {
		using namespace std;
		BoundingOrientedBox::CreateFromPoints(Out, Count, pPoints, Stride);
		XMMATRIX rot = XMMatrixIdentity();
		auto& ext = Out.Extents;

		// Sort the demension
		if (ext.x < ext.y)
		{
			swap(ext.x, ext.y);
			swap(rot.r[0], rot.r[1]);
			rot.r[2] = -rot.r[2]; // Flip the other axias to maintain the chirality
		}
		if (ext.y < ext.z)
		{
			swap(ext.y, ext.z);
			swap(rot.r[1], rot.r[2]);
			rot.r[0] = -rot.r[0];
		}
		if (ext.x < ext.y)
		{
			swap(ext.x, ext.y);
			swap(rot.r[0], rot.r[1]);
			rot.r[2] = -rot.r[2];
		}

		//qw = sqrt(1 + m00 + m11 + m22) / 2
		//qx = (m21 - m12) / (4 * qw)
		//qy = (m02 - m20) / (4 * qw)
		//qz = (m10 - m01) / (4 * qw)
		XMVECTOR q = XMQuaternionRotationMatrix(rot);
		XMStoreFloat4(&Out.Orientation, XMQuaternionMultiply(q, XMLoadFloat4(&Out.Orientation)));
		//auto q = XMQuaternionRotationMatrix(rot);
	}

	inline XMVECTOR CaculatePrincipleAxisFromPoints(_In_ size_t Count,
		_In_reads_bytes_(sizeof(XMFLOAT3) + Stride*(Count - 1)) const XMFLOAT3* pPoints, _In_ size_t Stride, float pointsUpperBound);

	// Create AABB & OBB from points, also normalize points extents to prevent float overflow during caculating principle axis
	inline void CreateBoundingBoxesFromPoints(_Out_ BoundingBox& aabb, _Out_ BoundingOrientedBox& obb, _In_ size_t Count,
		_In_reads_bytes_(sizeof(XMFLOAT3) + Stride*(Count - 1)) const XMFLOAT3* pPoints, _In_ size_t Stride);

	inline XMMATRIX XM_CALLCONV XMMatrixScalingFromCenter(FXMVECTOR scale, FXMVECTOR center);

	// No Matrix Multiply inside, significant faster than affine transform
	inline XMMATRIX XM_CALLCONV XMMatrixRigidTransform(FXMVECTOR RotationOrigin, FXMVECTOR RotationQuaterion, FXMVECTOR Translation);

	// No Matrix Multiply inside, significant faster than affine transform
	inline XMMATRIX XM_CALLCONV XMMatrixRigidTransform(FXMVECTOR RotationQuaterion, FXMVECTOR Translation);

#pragma region XMLoadStoreA Helpers
	inline XMVECTOR XMLoad(const SimpleMath::Vector3& src)
	{
		return XMLoadFloat3(&src);
	}
	inline XMVECTOR XMLoadA(const SimpleMath::Vector3& src)
	{
		return XMLoadFloat3A(reinterpret_cast<const XMFLOAT3A*>(&src));
	}
	inline void XM_CALLCONV XMStore(SimpleMath::Vector3& dest, FXMVECTOR v)
	{
		XMStoreFloat3(&dest, v);
	}
	inline void XM_CALLCONV XMStoreA(SimpleMath::Vector3& dest, FXMVECTOR v)
	{
		XMStoreFloat3A(reinterpret_cast<XMFLOAT3A*>(&dest), v);
	}
	inline XMVECTOR XMLoad(const SimpleMath::Vector4& src)
	{
		return XMLoadFloat4(&src);
	}
	inline XMVECTOR XMLoadA(const SimpleMath::Vector4& src)
	{
		return XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&src));
	}
	inline void XM_CALLCONV XMStore(SimpleMath::Vector4& dest, FXMVECTOR v)
	{
		XMStoreFloat4(&dest, v);
	}
	inline void XM_CALLCONV XMStoreA(SimpleMath::Vector4& dest, FXMVECTOR v)
	{
		XMStoreFloat4A(reinterpret_cast<XMFLOAT4A*>(&dest), v);
	}
	inline XMVECTOR XMLoad(const SimpleMath::Quaternion& src)
	{
		return XMLoadFloat4(&src);
	}
	inline XMVECTOR XMLoadA(const SimpleMath::Quaternion& src)
	{
		return XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&src));
	}
	inline void XM_CALLCONV XMStore(SimpleMath::Quaternion& dest, FXMVECTOR v)
	{
		XMStoreFloat4(&dest, v);
	}
	inline void XM_CALLCONV XMStoreA(SimpleMath::Quaternion& dest, FXMVECTOR v)
	{
		XMStoreFloat4A(reinterpret_cast<XMFLOAT4A*>(&dest), v);
	}
	inline XMMATRIX XMLoad(const SimpleMath::Matrix& src)
	{
		return XMLoadFloat4x4(&src);
	}
	inline XMMATRIX XMLoadA(const SimpleMath::Matrix& src)
	{
		return XMLoadFloat4x4A(reinterpret_cast<const XMFLOAT4X4A*>(&src));
	}
	inline void XM_CALLCONV XMStore(SimpleMath::Matrix& dest, FXMMATRIX m)
	{
		XMStoreFloat4x4(&dest, m);
	}
	inline void XM_CALLCONV XMStoreA(SimpleMath::Matrix& dest, FXMMATRIX m)
	{
		XMStoreFloat4x4A(reinterpret_cast<XMFLOAT4X4A*>(&dest), m);
	}
#pragma endregion

	XM_ALIGNATTR
	struct RotationTransform : public DirectX::AlignedNew<XMVECTOR>
	{
		XM_ALIGNATTR
			Quaternion  Rotation;
		// Extract the Matrix Representation of this rigid transform
		inline XMMATRIX TransformMatrix() const
		{
			XMMATRIX M = XMMatrixRotationQuaternion(XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&Rotation)));
			return M;
		}

		RotationTransform() = default;

		RotationTransform(const Quaternion& q)
			: Rotation(q)
		{
		}

		operator XMVECTOR() const
		{
			return XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&Rotation));
		}

		inline void XM_CALLCONV SetFromTransformMatrix(FXMMATRIX transform)
		{
			XMVECTOR scl, rot, tra;
			XMMatrixDecompose(&scl, &rot, &tra, transform);
			//XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&Rotation))
			XMStoreFloat4A(reinterpret_cast<XMFLOAT4A*>(&Rotation), rot);
			//Rotation.StoreA(rot);
		}
	};
	XM_ALIGNATTR
	struct TranslationTransform : public DirectX::AlignedNew<XMVECTOR>
	{
		XM_ALIGNATTR
			Vector3  Translation;
		float Tw;
		// Extract the Matrix Representation of this rigid transform
		inline XMMATRIX TransformMatrix() const
		{
			XMMATRIX M = XMMatrixTranslationFromVector(XMLoadFloat3A(reinterpret_cast<const XMFLOAT3A*>(&Translation)));
			return M;
		}

		operator XMVECTOR() const
		{
			return XMLoadFloat3A(reinterpret_cast<const XMFLOAT3A*>(&Translation));
		}

		inline void XM_CALLCONV SetFromTransformMatrix(FXMMATRIX transform)
		{
			XMVECTOR scl, rot, tra;
			XMMatrixDecompose(&scl, &rot, &tra, transform);
			XMStoreFloat3A(reinterpret_cast<XMFLOAT3A*>(&Translation),tra);
			//Translation.StoreA(tra);
		}
	};
	XM_ALIGNATTR
	struct ScaleTransform : public DirectX::AlignedNew<XMVECTOR>
	{
		XM_ALIGNATTR
			Vector3 Scale;
		float Sw; // Padding		
		// Extract the Matrix Representation of this rigid transform
		inline XMMATRIX TransformMatrix() const
		{
			XMMATRIX M = XMMatrixScalingFromVector(XMLoadA(Scale));
			return M;
		}

		operator XMVECTOR() const
		{
			return XMLoadA(Scale);
		}

		inline void XM_CALLCONV SetFromTransformMatrix(FXMMATRIX transform)
		{
			XMVECTOR scl, rot, tra;
			XMMatrixDecompose(&scl, &rot, &tra, transform);
			XMStoreA(Scale,scl);
		}
	};

	XM_ALIGNATTR
	// Composition of Translation and Rotation
	struct RigidTransform : public DirectX::AlignedNew<XMVECTOR>
	{
	public:
		XM_ALIGNATTR
			Quaternion  Rotation;
		XM_ALIGNATTR
			Vector3		Translation;
		float		Tw; // padding

		RigidTransform()
		{}

		// Extract the Matrix Representation of this rigid transform
		inline XMMATRIX TransformMatrix() const
		{
			XMMATRIX M = XMMatrixRotationQuaternion(Rotation);
			XMVECTOR VTranslation = XMVectorSelect(g_XMSelect1110.v, Translation, g_XMSelect1110.v);
			M.r[3] = XMVectorAdd(M.r[3], VTranslation);
			return M;
		}

		inline void XM_CALLCONV SetFromTransformMatrix(FXMMATRIX transform)
		{
			XMVECTOR scl, rot, tra;
			XMMatrixDecompose(&scl, &rot, &tra, transform);
			Rotation = rot;
			Translation = tra;
		}

		template <typename _TTransform>
		RigidTransform& operator *=(const _TTransform& transform)
		{
			XMMATRIX mat = TransformMatrix();
			XMMATRIX mat2 = transform.TransformMatrix();
			mat *= mat2;
			SetFromTransformMatrix(mat);
			return *this;
		}

		// Rigid * Rigid
		template <>
		RigidTransform& operator *=(const RigidTransform& global)
		{
			auto& local = *this;
			XMVECTOR ParQ = XMLoadA(global.Rotation);
			XMVECTOR Q = XMQuaternionMultiply(XMLoadA(local.Rotation), ParQ);
			XMStoreA(this->Rotation,Q);

			XMVECTOR V = XMLoadA(local.Translation);
			V = XMVector3Rotate(V, ParQ);
			V = XMVectorAdd(V, XMLoadA(global.Translation));

			XMStoreA(this->Translation,V);
		}

		template <>
		RigidTransform& operator *=(const Quaternion& rot)
		{
			auto& local = *this;
			XMVECTOR ParQ = XMLoad(rot);
			XMVECTOR Q = XMQuaternionMultiply(XMLoadA(local.Rotation), ParQ);
			XMStoreA(this->Rotation,Q);

			XMVECTOR V = XMLoadA(local.Translation);
			V = XMVector3Rotate(V, ParQ);

			XMStoreA(this->Translation,V);
		}

		template <>
		RigidTransform& operator *=(const Vector3& trans)
		{
			this->Translation += trans;
		}

		// Extract the Dual-Quaternion Representation of this rigid transform
		inline XMDUALVECTOR TransformDualQuaternion() const
		{
			return XMDualQuaternionRotationTranslation(Rotation, Translation);
		}

		bool NearEqual(const RigidTransform& rhs, float tEpsilon = 0.002f, float rEpsilon = 0.5f) const
		{
			Vector3 PosDiff = Translation - rhs.Translation;
			XMVECTOR RotDiff = Rotation;
			RotDiff = XMQuaternionInverse(RotDiff);
			RotDiff = XMQuaternionMultiply(RotDiff, rhs.Rotation);
			float AngDiff = 2 * acosf(XMVectorGetW(RotDiff));
			return (PosDiff.LengthSquared() <= tEpsilon*tEpsilon && AngDiff <= rEpsilon);
		}
	};

	XM_ALIGNATTR
	// (Scale*)-Rotation-Translation
	//! SRT(RST) transform is not a 'Group' in term of MATRIX production
	//! Thus, We define SRT * SRT -> (S0*S1)*R0*T0*R1*T1
	//! A simple extension to rigid transform, but will apply well for uniform scaling
	struct ScaledRigidTransform : protected RigidTransform
	{
		using RigidTransform::Rotation;
		using RigidTransform::Translation;
		using RigidTransform::Tw;
		XM_ALIGNATTR
		Vector3 Scale;
		float Sw; // Padding

		static ScaledRigidTransform Identity()
		{
			return ScaledRigidTransform();
		}


		ScaledRigidTransform()
			: Scale(1.0f)
		{}

		inline explicit ScaledRigidTransform(CXMMATRIX transform)
		{
			SetFromTransformMatrix(transform);
		}

		inline explicit ScaledRigidTransform(const RigidTransform &rigid)
			: RigidTransform(rigid), Scale(1.0f)
		{
		}

		inline void XM_CALLCONV SetFromTransformMatrix(FXMMATRIX transform)
		{
			XMVECTOR scl, rot, tra;
			XMMatrixDecompose(&scl, &rot, &tra, transform);
			Scale = scl;
			Rotation = rot;
			Translation = tra;
		}

		inline void Inverse();

		inline ScaledRigidTransform Inversed() const;

		template <typename _TTransform>
		ScaledRigidTransform& operator *=(const _TTransform& transform)
		{
			XMMATRIX mat = TransformMatrix();
			XMMATRIX mat2 = transform.TransformMatrix();
			mat *= mat2;
			SetFromTransformMatrix(mat);
			return *this;
		}

		// SRT * SRT -> (S0*S1)*R0*T0*R1*T1
		// caculate the transform of Local transform 'this' conacting with Global transform 'rhs'
		template <>
		ScaledRigidTransform& operator *=(const ScaledRigidTransform& global)
		{
			auto& local = *this;
			XMVECTOR ParQ = XMLoadA(global.Rotation);
			XMVECTOR Q = XMQuaternionMultiply(XMLoadA(local.Rotation), ParQ);
			XMVECTOR ParS = XMLoadA(global.Scale);
			XMVECTOR S = ParS*XMLoadA(local.Scale);
			XMStoreA(this->Rotation,Q);
			XMStoreA(this->Scale,S);

			XMVECTOR V = XMLoadA(local.Translation);
			V *= ParS;
			V = XMVector3Rotate(V, ParQ);
			V = XMVectorAdd(V, XMLoadA(global.Translation));

			XMStoreA(this->Translation,V);
			return *this;
		}

		template <>
		ScaledRigidTransform& operator *=(const ScaleTransform& global)
		{
			auto& local = *this;
			XMVECTOR ParS = XMLoadA(global.Scale);
			XMVECTOR S = ParS * XMLoadA(local.Scale);
			XMStoreA(this->Scale,S);

			XMVECTOR V = XMLoadA(local.Translation);
			V *= ParS;
			XMStoreA(this->Translation,V);
			return *this;
		}


		// ScaledRigid * Rigid
		template <>
		ScaledRigidTransform& operator *=(const RigidTransform& global)
		{
			auto& local = *this;
			XMVECTOR ParQ = XMLoadA(global.Rotation);
			XMVECTOR Q = XMQuaternionMultiply(XMLoadA(local.Rotation), ParQ);
			XMStoreA(this->Rotation,Q);

			XMVECTOR V = XMLoadA(local.Translation);
			V = XMVector3Rotate(V, ParQ);
			V = XMVectorAdd(V, XMLoadA(global.Translation));

			XMStoreA(this->Translation,V);
			return *this;
		}

		template <>
		ScaledRigidTransform& operator *=(const Quaternion& globalRot)
		{
			auto& local = *this;
			XMVECTOR ParQ = XMLoad(globalRot);
			XMVECTOR Q = XMQuaternionMultiply(XMLoadA(local.Rotation), ParQ);
			XMStoreA(this->Rotation,Q);

			XMVECTOR V = XMLoadA(local.Translation);
			V = XMVector3Rotate(V, ParQ);

			XMStoreA(this->Translation,V);
			return *this;
		}

		template <>
		ScaledRigidTransform& operator *=(const RotationTransform& global)
		{
			auto& local = *this;
			XMVECTOR ParQ = XMLoadA(global.Rotation);
			XMVECTOR Q = XMQuaternionMultiply(XMLoadA(local.Rotation), ParQ);
			XMStoreA(this->Rotation,Q);

			XMVECTOR V = XMLoadA(local.Translation);
			V = XMVector3Rotate(V, ParQ);

			XMStoreA(this->Translation,V);
			return *this;
		}

		template <>
		ScaledRigidTransform& operator *=(const TranslationTransform& global)
		{
			this->Translation += global.Translation;
			return *this;
		}

		inline XMMATRIX TransformMatrix() const
		{
			XMMATRIX M = XMMatrixScalingFromVector(Scale);
			M *= XMMatrixRotationQuaternion(Rotation);
			XMVECTOR VTranslation = XMVectorSelect(g_XMSelect1110.v, Translation, g_XMSelect1110.v);
			M.r[3] = XMVectorAdd(M.r[3], VTranslation);
			return M;
		}

		explicit operator XMMATRIX() const
		{
			return TransformMatrix();
		}
	};

	struct MatrixTransform : public Matrix4x4
	{
		using Matrix4x4::operator();
		using Matrix4x4::operator+=;
		using Matrix4x4::operator-=;
		using Matrix4x4::operator*=;
		using Matrix4x4::operator/=;
		using Matrix4x4::operator-;
		using Matrix4x4::operator DirectX::XMMATRIX;

		inline void XM_CALLCONV SetFromTransformMatrix(FXMMATRIX transform)
		{
			*this = transform;
		}

		MatrixTransform& operator=(const Matrix4x4& rhs)
		{
			Matrix4x4::operator=(rhs);
		}

		template <typename _TTransform>
		MatrixTransform& operator *=(const _TTransform& transform)
		{
			XMMATRIX mat = TransformMatrix();
			XMMATRIX mat2 = transform.TransformMatrix();
			mat *= mat2;
			SetFromTransformMatrix(mat);
			return *this;
		}

		inline XMMATRIX TransformMatrix() const
		{
			return XMLoadFloat4x4(this);
		}
	};

	enum BoundingGeometryType
	{
		Geometry_Unknown,
		Geometry_AxisAlignedBox,
		Geometry_OrientedBox,
		Geometry_Frustum,
		Geometry_Sphere,
	};

	struct BoundingGeometry
	{
		BoundingGeometryType Type;
		union
		{
			BoundingBox				AxisAlignedBox;
			BoundingOrientedBox		OrientedBox;
			BoundingFrustum			Frustum;
			BoundingSphere			Sphere;
		};

		BoundingGeometry();

		explicit BoundingGeometry(const BoundingBox& aabb);
		explicit BoundingGeometry(const BoundingOrientedBox& obox);
		explicit BoundingGeometry(const BoundingFrustum& frustum);
		explicit BoundingGeometry(const BoundingSphere& sphere);

		BoundingGeometry& operator=(const BoundingGeometry& rhs) = default;
		BoundingGeometry& operator=(BoundingGeometry&& rhs) = default;
		BoundingGeometry& operator=(const BoundingBox& rhs);
		BoundingGeometry& operator=(const BoundingOrientedBox& rhs);
		BoundingGeometry& operator=(const BoundingFrustum& rhs);
		BoundingGeometry& operator=(const BoundingSphere& rhs);

		ContainmentType XM_CALLCONV Contains(_In_ FXMVECTOR Point) const;
		ContainmentType XM_CALLCONV Contains(_In_ FXMVECTOR V0, _In_ FXMVECTOR V1, _In_ FXMVECTOR V2) const;

		template <typename GeometryType>
		ContainmentType ContainsGeometry(_In_ const GeometryType& geometry) const;

		ContainmentType Contains(_In_ const BoundingSphere& geometry) const;
		ContainmentType Contains(_In_ const BoundingBox& geometry) const;
		ContainmentType Contains(_In_ const BoundingOrientedBox& geometry) const;
		ContainmentType Contains(_In_ const BoundingFrustum& geometry) const;
		ContainmentType Contains(_In_ const BoundingGeometry& geometry) const;

		// Six-Plane containment test
		ContainmentType XM_CALLCONV ContainedBy(_In_ FXMVECTOR Plane0, _In_ FXMVECTOR Plane1, _In_ FXMVECTOR Plane2,
			_In_ GXMVECTOR Plane3, _In_ HXMVECTOR Plane4, _In_ HXMVECTOR Plane5) const;

		template <typename GeometryType>
		bool IntersectsGeometry(_In_ const GeometryType& geometry) const;

		bool Intersects(_In_ const BoundingSphere& geometry) const;
		bool Intersects(_In_ const BoundingBox& geometry) const;
		bool Intersects(_In_ const BoundingOrientedBox& geometry) const;
		bool Intersects(_In_ const BoundingFrustum& geometry) const;
		bool Intersects(_In_ const BoundingGeometry& geometry) const;

		// Triangle test
		bool XM_CALLCONV Intersects(_In_ FXMVECTOR V0, _In_ FXMVECTOR V1, _In_ FXMVECTOR V2) const;
		// Ray test
		bool XM_CALLCONV Intersects(_In_ FXMVECTOR Origin, _In_ FXMVECTOR Direction, _Out_ float& Dist) const;
		// Plane test
		PlaneIntersectionType XM_CALLCONV	Intersects(_In_ FXMVECTOR Plane) const;

		void XM_CALLCONV Transform(_Out_ BoundingGeometry& Out, _In_ FXMMATRIX M) const;
		void XM_CALLCONV Transform(_Out_ BoundingGeometry& Out, _In_ float Scale, _In_ FXMVECTOR Rotation, _In_ FXMVECTOR Translation) const;
	};


	namespace BoundingFrustumExtension
	{
		inline void CreateFromMatrixRH(BoundingFrustum& Out, CXMMATRIX Projection);
	}

	//Some supporting method

	namespace LineSegmentTest
	{
		inline float XM_CALLCONV Distance(FXMVECTOR p, FXMVECTOR s0, FXMVECTOR s1);
		inline float XM_CALLCONV Distance(FXMVECTOR p, const XMFLOAT3 *path, size_t nPoint, size_t strideInByte = sizeof(XMFLOAT3));

		// Takes a space point and space line segment , return the projection point on the line segment
		//  A0  |		A1		  |
		//      |s0-------------s1|
		//      |				  |		A2
		// where p in area of A0 returns s0 , area A2 returns s1 , point in A1 returns the really projection point 
		inline XMVECTOR XM_CALLCONV Projection(FXMVECTOR p, FXMVECTOR s0, FXMVECTOR s1);

		template <typename T>
		inline XMVECTOR XM_CALLCONV Projection(FXMVECTOR p, const T *path, size_t nPoint, size_t strideInByte = sizeof(T));
	}

}

#include "DirectXMathExtend.inl"

// Extending std lib for output
#ifdef _OSTREAM_
#include <iomanip>
namespace std
{
	inline std::ostream& operator << (std::ostream& lhs, const DirectX::XMFLOAT2& rhs)
	{
		lhs << '(' << std::setw(6) << setiosflags(std::ios::fixed) << std::setprecision(3) << rhs.x
			<< "," << std::setw(6) << setiosflags(std::ios::fixed) << std::setprecision(3) << rhs.y << ')';
		return lhs;
	};

	inline std::ostream& operator << (std::ostream& lhs, const DirectX::XMFLOAT3& rhs)
	{
		lhs << '(' << std::setw(6) << setiosflags(std::ios::fixed) << std::setprecision(3) << rhs.x
			<< "," << std::setw(6) << setiosflags(std::ios::fixed) << std::setprecision(3) << rhs.y
			<< "," << std::setw(6) << setiosflags(std::ios::fixed) << std::setprecision(3) << rhs.z << ')';
		return lhs;
	};

	inline std::ostream& operator << (std::ostream& lhs, const DirectX::XMFLOAT4& rhs)
	{
		lhs << '(' << std::setw(6) << setiosflags(std::ios::fixed) << std::setprecision(3) << rhs.x
			<< "," << std::setw(6) << setiosflags(std::ios::fixed) << std::setprecision(3) << rhs.y
			<< "," << std::setw(6) << setiosflags(std::ios::fixed) << std::setprecision(3) << rhs.z
			<< "," << std::setw(6) << setiosflags(std::ios::fixed) << std::setprecision(3) << rhs.w << ')';
		return lhs;
	};

	inline std::ostream& operator << (std::ostream& lhs, const DirectX::Quaternion& rhs)
	{
		float theta = std::acosf(rhs.w) * 2 / DirectX::XM_PI;
		DirectX::Vector3 axis(rhs);
		axis.Normalize();
		lhs << '(' << axis
			<< "," << std::setw(6) << setiosflags(std::ios::fixed) << std::setprecision(3) << theta << "*Pi)";
		return lhs;
	};
}
#endif

#endif