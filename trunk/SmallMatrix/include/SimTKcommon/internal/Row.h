#ifndef SimTK_SIMMATRIX_SMALLMATRIX_ROW_H_
#define SimTK_SIMMATRIX_SMALLMATRIX_ROW_H_

/* Portions copyright (c) 2005-6 Stanford University and Michael Sherman.
 * Contributors:
 * 
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including 
 * without limitation the rights to use, copy, modify, merge, publish, 
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included 
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**@file
 * This file declares class Row<NCOLS, ELEMENT_TYPE, STRIDE>.
 */

#include "SimTKcommon/internal/common.h"


namespace SimTK {


/// Generic Row
template <int N, class ELT, int STRIDE> class Row {
    typedef ELT                         E;
    typedef typename CNT<E>::TNeg       ENeg;
    typedef typename CNT<E>::TAbs       EAbs;
    typedef typename CNT<E>::TStandard  EStandard;
    typedef typename CNT<E>::TReal      EReal;
    typedef typename CNT<E>::TImag      EImag;
    typedef typename CNT<E>::TComplex   EComplex;
    typedef typename CNT<E>::THerm      EHerm;
    typedef typename CNT<E>::TInvert    EInvert;
    typedef typename CNT<E>::TPosTrans  EPosTrans;
    typedef typename CNT<E>::TSqHermT   ESqHermT;
    typedef typename CNT<E>::TSqTHerm   ESqTHerm;

    typedef typename CNT<E>::Scalar     EScalar;
    typedef typename CNT<E>::Number     ENumber;
    typedef typename CNT<E>::StdNumber  EStdNumber;
    typedef typename CNT<E>::Precision  EPrecision;
    typedef typename CNT<E>::ScalarSq   EScalarSq;

public:

    enum {
        NRows               = 1,
        NCols               = N,
        RowSpacing          = 1,
        ColSpacing          = STRIDE,
        NPackedElements     = N,
        NActualElements     = N * STRIDE,   // includes trailing gap
        NActualScalars      = CNT<E>::NActualScalars * NActualElements,
        ImagOffset          = NTraits<ENumber>::ImagOffset,
        RealStrideFactor    = 1, // composite types don't change size when
                                 // cast from complex to real or imaginary
        ArgDepth            = ((int)CNT<E>::ArgDepth < (int)MAX_RESOLVED_DEPTH 
                                ? CNT<E>::ArgDepth + 1 
                                : MAX_RESOLVED_DEPTH),
        IsScalar            = 0,
        IsNumber            = 0,
        IsStdNumber         = 0,
        IsPrecision         = 0
    };

    typedef Row<N,E,STRIDE>             T;
    typedef Row<N,ENeg,STRIDE>          TNeg;
    typedef Row<N,EAbs,1>               TAbs;       // Note stride
    typedef Row<N,EStandard,1>          TStandard;
    typedef Row<N,EReal,STRIDE*CNT<E>::RealStrideFactor>         
                                        TReal;
    typedef Row<N,EImag,STRIDE*CNT<E>::RealStrideFactor>         
                                        TImag;
    typedef Row<N,EComplex,STRIDE>      TComplex;
    typedef Vec<N,EHerm,STRIDE>         THerm;
    typedef Vec<N,EInvert,1>            TInvert;    // packed
    typedef Vec<N,E,STRIDE>             TPosTrans;
    typedef SymMat<N,ESqHermT>          TSqHermT;   // result of self outer product
    typedef EScalarSq                   TSqTHerm;   // result of self dot product
    typedef E                           TElement;
    typedef Row                         TRow;
    typedef E                           TCol;

    typedef EScalar                     Scalar;
    typedef ENumber                     Number;
    typedef EStdNumber                  StdNumber;
    typedef EPrecision                  Precision;
    typedef EScalarSq                   ScalarSq;

    int size()   const  { return N; }
    int nrow()   const  { return 1; }
    int ncol()   const  { return N; }


    // Scalar norm square is sum( squares of all scalars )
    ScalarSq scalarNormSqr() const { 
        ScalarSq sum(0);
        for(int i=0;i<N;++i) sum += CNT<E>::scalarNormSqr(d[i*STRIDE]);
        return sum;
    }

    // abs() is elementwise absolute value; that is, the return value has the same
    // dimension as this Row but with each element replaced by whatever it thinks
    // its absolute value is.
    TAbs abs() const {
        TAbs rabs;
        for(int i=0;i<N;++i) rabs[i] = CNT<E>::abs(d[i*STRIDE]);
        return rabs;
    }

    TStandard standardize() const {
        TStandard rstd;
        for(int i=0;i<N;++i) rstd[i] = CNT<E>::standardize(d[i*STRIDE]);
        return rstd;
    }

    // Sum just adds up all the elements, getting rid of negators and
    // conjugates in the process.
    EStandard sum() const {
        E sum(0);
        for (int i=0;i<N;++i) sum += d[i*STRIDE];
        return CNT<E>::standardize(sum);
    }

    // This gives the resulting rowvector type when (v[i] op P) is applied to each element of v.
    // It is a row of length N, stride 1, and element types which are the regular
    // composite result of E op P. Typically P is a scalar type but it doesn't have to be.
    template <class P> struct EltResult { 
        typedef Row<N, typename CNT<E>::template Result<P>::Mul, 1> Mul;
        typedef Row<N, typename CNT<E>::template Result<P>::Dvd, 1> Dvd;
        typedef Row<N, typename CNT<E>::template Result<P>::Add, 1> Add;
        typedef Row<N, typename CNT<E>::template Result<P>::Sub, 1> Sub;
    };

    // This is the composite result for v op P where P is some kind of appropriately shaped
    // non-scalar type.
    template <class P> struct Result { 
        typedef MulCNTs<1,N,ArgDepth,Row,ColSpacing,RowSpacing,
            CNT<P>::NRows, CNT<P>::NCols, CNT<P>::ArgDepth,
            P, CNT<P>::ColSpacing, CNT<P>::RowSpacing> MulOp;
        typedef typename MulOp::Type Mul;

        typedef MulCNTsNonConforming<1,N,ArgDepth,Row,ColSpacing,RowSpacing,
            CNT<P>::NRows, CNT<P>::NCols, CNT<P>::ArgDepth,
            P, CNT<P>::ColSpacing, CNT<P>::RowSpacing> MulOpNonConforming;
        typedef typename MulOpNonConforming::Type MulNon;


        typedef DvdCNTs<1,N,ArgDepth,Row,ColSpacing,RowSpacing,
            CNT<P>::NRows, CNT<P>::NCols, CNT<P>::ArgDepth,
            P, CNT<P>::ColSpacing, CNT<P>::RowSpacing> DvdOp;
        typedef typename DvdOp::Type Dvd;

        typedef AddCNTs<1,N,ArgDepth,Row,ColSpacing,RowSpacing,
            CNT<P>::NRows, CNT<P>::NCols, CNT<P>::ArgDepth,
            P, CNT<P>::ColSpacing, CNT<P>::RowSpacing> AddOp;
        typedef typename AddOp::Type Add;

        typedef SubCNTs<1,N,ArgDepth,Row,ColSpacing,RowSpacing,
            CNT<P>::NRows, CNT<P>::NCols, CNT<P>::ArgDepth,
            P, CNT<P>::ColSpacing, CNT<P>::RowSpacing> SubOp;
        typedef typename SubOp::Type Sub;
    };

    // Shape-preserving element substitution (always packed)
    template <class P> struct Substitute {
        typedef Row<N,P> Type;
    };

    // Default construction initializes to NaN when debugging but
    // is left uninitialized otherwise.
	Row(){ 
    #ifndef NDEBUG
        setToNaN();
    #endif
    }

    // It's important not to use the default copy constructor or copy
    // assignment because the compiler doesn't understand that we may
    // have noncontiguous storage and will try to copy the whole array.
    Row(const Row& src) {
        for (int i=0; i<N; ++i)
            d[i*STRIDE] = src[i];
    }
    Row& operator=(const Row& src) {    // no harm if src and 'this' are the same
        for (int i=0; i<N; ++i)
            d[i*STRIDE] = src[i];
        return *this;
    }

    // We want an implicit conversion from a Row of the same length
    // and element type but with a different stride.
    template <int SS> Row(const Row<N,E,SS>& src) {
        for (int i=0; i<N; ++i)
            d[i*STRIDE] = src[i];
    }

    // We want an implicit conversion from a Row of the same length
    // and *negated* element type, possibly with a different stride.
    template <int SS> Row(const Row<N,ENeg,SS>& src) {
        for (int i=0; i<N; ++i)
            d[i*STRIDE] = src[i];
    }

    // Construct a Row from a Row of the same length, with any
    // stride. Works as long as the element types are compatible.
    template <class EE, int SS> explicit Row(const Row<N,EE,SS>& vv)
      { for (int i=0;i<N;++i) d[i*STRIDE]=vv[i]; }

    // Construction using an element assigns to each element.
    explicit Row(const ELT& e)
      { for (int i=0;i<N;++i) d[i*STRIDE]=e; }

    // A bevy of constructors for Rows up to length 6.
    Row(const E& e0,const E& e1)
      { assert(N==2);(*this)[0]=e0;(*this)[1]=e1; }
    Row(const E& e0,const E& e1,const E& e2)
      { assert(N==3);(*this)[0]=e0;(*this)[1]=e1;(*this)[2]=e2; }
    Row(const E& e0,const E& e1,const E& e2,const E& e3)
      { assert(N==4);(*this)[0]=e0;(*this)[1]=e1;(*this)[2]=e2;(*this)[3]=e3; }
    Row(const E& e0,const E& e1,const E& e2,const E& e3,const E& e4)
      { assert(N==5);(*this)[0]=e0;(*this)[1]=e1;(*this)[2]=e2;
        (*this)[3]=e3;(*this)[4]=e4; }
    Row(const E& e0,const E& e1,const E& e2,const E& e3,const E& e4,const E& e5)
      { assert(N==6);(*this)[0]=e0;(*this)[1]=e1;(*this)[2]=e2;
        (*this)[3]=e3;(*this)[4]=e4;(*this)[5]=e5; }

    // Construction from a pointer to anything assumes we're pointing
    // at an element list of the right length.
    template <class EE> explicit Row(const EE* p)
      { assert(p); for(int i=0;i<N;++i) d[i*STRIDE]=p[i]; }
    template <class EE> Row& operator=(const EE* p)
      { assert(p); for(int i=0;i<N;++i) d[i*STRIDE]=p[i]; return *this; }

    // Conforming assignment ops.
    template <class EE, int SS> Row& operator=(const Row<N,EE,SS>& vv)
      { for(int i=0;i<N;++i) d[i*STRIDE] = vv[i]; return *this; }
    template <class EE, int SS> Row& operator+=(const Row<N,EE,SS>& r)
      { for(int i=0;i<N;++i) d[i*STRIDE] += r[i]; return *this; }
    template <class EE, int SS> Row& operator+=(const Row<N,negator<EE>,SS>& r)
      { for(int i=0;i<N;++i) d[i*STRIDE] -= -(r[i]); return *this; }
    template <class EE, int SS> Row& operator-=(const Row<N,EE,SS>& r)
      { for(int i=0;i<N;++i) d[i*STRIDE] -= r[i]; return *this; }
    template <class EE, int SS> Row& operator-=(const Row<N,negator<EE>,SS>& r)
      { for(int i=0;i<N;++i) d[i*STRIDE] += -(r[i]); return *this; }

    // Conforming binary ops with 'this' on left, producing new packed result.
    // Cases: r=r+r, r=r-r, s=r*v r=r*m
    template <class EE, int SS> Row<N,typename CNT<E>::template Result<EE>::Add>
    conformingAdd(const Row<N,EE,SS>& r) const {
        Row<N,typename CNT<E>::template Result<EE>::Add> result;
        for (int j=0;j<N;++j) result[j] = (*this)[j] + r[j];
        return result;
    }
    template <class EE, int SS> Row<N,typename CNT<E>::template Result<EE>::Sub>
    conformingSubtract(const Row<N,EE,SS>& r) const {
        Row<N,typename CNT<E>::template Result<EE>::Sub> result;
        for (int j=0;j<N;++j) result[j] = (*this)[j] - r[j];
        return result;
    }

    // dot product
    template <class EE, int SS> typename CNT<E>::template Result<EE>::Mul
    conformingMultiply(const Vec<N,EE,SS>& r) const {
        typename CNT<E>::template Result<EE>::Mul result(0);
        for (int j=0;j<N;++j) result += (*this)[j]*r[j];
        return result;
    }

    // row=row*mat
    template <int MatNCol, class EE, int CS, int RS> 
    Row<MatNCol,typename CNT<E>::template Result<EE>::Mul>
    conformingMultiply(const Mat<N,MatNCol,EE,CS,RS>& m) const {
        Row<MatNCol,typename CNT<E>::template Result<EE>::Mul> result;
        for (int j=0;j<N;++j) result[j] = conformingMultiply(m(j));
        return result;
    }

	const E& operator[](int i) const { assert(0 <= i && i < N); return d[i*STRIDE]; }
	E&       operator[](int i)	     { assert(0 <= i && i < N); return d[i*STRIDE]; }
    const E& operator()(int i) const { return (*this)[i]; }
	E&       operator()(int i)	     { return (*this)[i]; }

    ScalarSq normSqr() const { return scalarNormSqr(); }
    ScalarSq norm()    const { return std::sqrt(scalarNormSqr()); }

    TInvert invert() const {assert(false); return TInvert();} // TODO default inversion

    const Row&   operator+() const { return *this; }
    const TNeg&  operator-() const { return negate(); }
    TNeg&        operator-()       { return updNegate(); }
    const THerm& operator~() const { return transpose(); }
    THerm&       operator~()       { return updTranspose(); }

    const TNeg&  negate() const { return *reinterpret_cast<const TNeg*>(this); }
    TNeg&        updNegate()    { return *reinterpret_cast<TNeg*>(this); }

    const THerm& transpose()    const { return *reinterpret_cast<const THerm*>(this); }
    THerm&       updTranspose()       { return *reinterpret_cast<THerm*>(this); }

    const TPosTrans& positionalTranspose() const
        { return *reinterpret_cast<const TPosTrans*>(this); }
    TPosTrans&       updPositionalTranspose()
        { return *reinterpret_cast<TPosTrans*>(this); }

    const TReal& real() const { return *reinterpret_cast<const TReal*>(this); }
    TReal&       real()       { return *reinterpret_cast<      TReal*>(this); }

    // Had to contort these routines to get them through VC++ 7.net
    const TImag& imag()    const { 
        const int offs = ImagOffset;
        const EImag* p = reinterpret_cast<const EImag*>(this);
        return *reinterpret_cast<const TImag*>(p+offs);
    }
    TImag& imag() { 
        const int offs = ImagOffset;
        EImag* p = reinterpret_cast<EImag*>(this);
        return *reinterpret_cast<TImag*>(p+offs);
    }

    // These are elementwise binary operators, (this op ee) by default but (ee op this) if
    // 'FromLeft' appears in the name. The result is a packed Row<N> but the element type
    // may change. These are mostly used to implement global operators.

    //TODO: consider converting 'e' to Standard Numbers as precalculation and changing
    // return type appropriately.
    template <class EE> Row<N, typename CNT<E>::template Result<EE>::Mul>
    scalarMultiply(const EE& e) const {
        Row<N, typename CNT<E>::template Result<EE>::Mul> result;
        for (int j=0; j<N; ++j) result[j] = (*this)[j] * e;
        return result;
    }
    template <class EE> Row<N, typename CNT<EE>::template Result<E>::Mul>
    scalarMultiplyFromLeft(const EE& e) const {
        Row<N, typename CNT<EE>::template Result<E>::Mul> result;
        for (int j=0; j<N; ++j) result[j] = e * (*this)[j];
        return result;
    }

    // TODO: should precalculate and store 1/e, while converting to Standard Numbers. Note
    // that return type should change appropriately.
    template <class EE> Row<N, typename CNT<E>::template Result<EE>::Dvd>
    scalarDivide(const EE& e) const {
        Row<N, typename CNT<E>::template Result<EE>::Dvd> result;
        for (int j=0; j<N; ++j) result[j] = (*this)[j] / e;
        return result;
    }
    template <class EE> Row<N, typename CNT<EE>::template Result<E>::Dvd>
    scalarDivideFromLeft(const EE& e) const {
        Row<N, typename CNT<EE>::template Result<E>::Dvd> result;
        for (int j=0; j<N; ++j) result[j] = e / (*this)[j];
        return result;
    }

    template <class EE> Row<N, typename CNT<E>::template Result<EE>::Add>
    scalarAdd(const EE& e) const {
        Row<N, typename CNT<E>::template Result<EE>::Add> result;
        for (int j=0; j<N; ++j) result[j] = (*this)[j] + e;
        return result;
    }
    // Add is commutative, so no 'FromLeft'.

    template <class EE> Row<N, typename CNT<E>::template Result<EE>::Sub>
    scalarSubtract(const EE& e) const {
        Row<N, typename CNT<E>::template Result<EE>::Sub> result;
        for (int j=0; j<N; ++j) result[j] = (*this)[j] - e;
        return result;
    }
    template <class EE> Row<N, typename CNT<EE>::template Result<E>::Sub>
    scalarSubtractFromLeft(const EE& e) const {
        Row<N, typename CNT<EE>::template Result<E>::Sub> result;
        for (int j=0; j<N; ++j) result[j] = e - (*this)[j];
        return result;
    }

    // Generic assignments for any element type not listed explicitly, including scalars.
    // These are done repeatedly for each element and only work if the operation can
    // be performed leaving the original element type.
    template <class EE> Row& operator =(const EE& e) {return scalarEq(e);}
    template <class EE> Row& operator+=(const EE& e) {return scalarPlusEq(e);}
    template <class EE> Row& operator-=(const EE& e) {return scalarMinusEq(e);}
    template <class EE> Row& operator*=(const EE& e) {return scalarTimesEq(e);}
    template <class EE> Row& operator/=(const EE& e) {return scalarDivideEq(e);}

    // Generalized scalar assignment & computed assignment methods. These will work
    // for any assignment-compatible element, not just scalars.
    template <class EE> Row& scalarEq(const EE& ee)
      { for(int i=0;i<N;++i) d[i*STRIDE] = ee; return *this; }
    template <class EE> Row& scalarPlusEq(const EE& ee)
      { for(int i=0;i<N;++i) d[i*STRIDE] += ee; return *this; }
    template <class EE> Row& scalarMinusEq(const EE& ee)
      { for(int i=0;i<N;++i) d[i*STRIDE] -= ee; return *this; }
    template <class EE> Row& scalarInverseMinusEq(const EE& ee)
      { for(int i=0;i<N;++i) d[i*STRIDE] = ee - d[i*STRIDE]; return *this; }
    template <class EE> Row& scalarTimesEq(const EE& ee)
      { for(int i=0;i<N;++i) d[i*STRIDE] *= ee; return *this; }
    template <class EE> Row& scalarDivideEq(const EE& ee)
      { for(int i=0;i<N;++i) d[i*STRIDE] /= ee; return *this; }
    template <class EE> Row& scalarInverseDivideEq(const EE& ee)
      { for(int i=0;i<N;++i) d[i*STRIDE] = ee / d[i*STRIDE]; return *this; }

    void setToNaN() {
        (*this) = CNT<ELT>::getNaN();
    }

    // Extract a sub-Row with size known at compile time. These have to be
    // called with explicit template arguments, e.g. getSubRow<3>(j).
    template <int NN>
    const Row<NN,ELT,STRIDE>& getSubRow(int j) const {
        assert(0 <= j && j + NN <= N);
        return Row<NN,ELT,STRIDE>::getAs(&(*this)[j]);
    }
    template <int NN>
    Row<NN,ELT,STRIDE>& updSubRow(int j) {
        assert(0 <= j && j + NN <= N);
        return Row<NN,ELT,STRIDE>::updAs(&(*this)[j]);
    }

    // Return a row one smaller than this one by dropping the element
    // at the indicated position p. The result is packed but has same
    // element type as this one.
    Row<N-1,ELT,1> drop1(int p) const {
        assert(0 <= p && p < N);
        Row<N-1,ELT,1> out;
        int nxt=0;
        for (int i=0; i<N-1; ++i, ++nxt) {
            if (nxt==p) ++nxt;  // skip the loser
            out[i] = (*this)[nxt];
        }
        return out;
    }

    // Return a vector one larger than this one by adding an element
    // to the end. The result is packed but has same element type as
    // this one. Works for any assignment compatible element.
    template <class EE> Row<N+1,ELT,1> append1(const EE& v) const {
        Row<N+1,ELT,1> out;
        Row<N,ELT,1>::updAs(&out[0]) = (*this);
        out[N] = v;
        return out;
    }


    // Return a vector one larger than this one by inserting an element
    // *before* the indicated one. The result is packed but has same element type as
    // this one. Works for any assignment compatible element. The index
    // can be one greater than normally allowed in which case the element
    // is appended.
    template <class EE> Row<N+1,ELT,1> insert1(int p, const EE& v) const {
        assert(0 <= p && p <= N);
        if (p==N) return append1(v);
        Row<N+1,ELT,1> out;
        int nxt=0;
        for (int i=0; i<N; ++i, ++nxt) {
            if (i==p) out[nxt++] = v;
            out[nxt] = (*this)[i];
        }
        return out;
    }

    // These assume we are given a pointer to d[0] of a Row<N,E,S> like this one.
    static const Row& getAs(const ELT* p)  {return *reinterpret_cast<const Row*>(p);}
    static Row&       updAs(ELT* p)        {return *reinterpret_cast<Row*>(p);}

    // Extract a subrow from a longer one. Element type and stride must match.
    template <int NN>
    static const Row& getSubRow(const Row<NN,ELT,STRIDE>& r, int j) {
        assert(0 <= j && j + N <= NN);
        return getAs(&r[j]);
    }
    template <int NN>
    static Row& updSubRow(Row<NN,ELT,STRIDE>& r, int j) {
        assert(0 <= j && j + N <= NN);
        return updAs(&r[j]);
    }

    static Row<N,ELT,1> getNaN() { return Row<N,ELT,1>(CNT<ELT>::getNaN()); }
private:
	ELT d[NActualElements];    // data
};

/////////////////////////////////////////////
// Global operators involving two rows.    //
//   v+v, v-v, v==v, v!=v                  //
/////////////////////////////////////////////

// v3 = v1 + v2 where all v's have the same length N. 
template <int N, class E1, int S1, class E2, int S2> inline
typename Row<N,E1,S1>::template Result< Row<N,E2,S2> >::Add
operator+(const Row<N,E1,S1>& l, const Row<N,E2,S2>& r) { 
    return Row<N,E1,S1>::template Result< Row<N,E2,S2> >
        ::AddOp::perform(l,r);
}

// v3 = v1 - v2, similar to +
template <int N, class E1, int S1, class E2, int S2> inline
typename Row<N,E1,S1>::template Result< Row<N,E2,S2> >::Sub
operator-(const Row<N,E1,S1>& l, const Row<N,E2,S2>& r) { 
    return Row<N,E1,S1>::template Result< Row<N,E2,S2> >
        ::SubOp::perform(l,r);
}

// bool = v1 == v2, v1 and v2 have the same length M
template <int N, class E1, int S1, class E2, int S2> inline bool
operator==(const Row<N,E1,S1>& l, const Row<N,E2,S2>& r) { 
    for (int i=0; i < N; ++i)
        if (l[i] != r[i]) return false;
    return true;
}

// bool = v1 != v2, v1 and v2 have the same length M
template <int N, class E1, int S1, class E2, int S2> inline bool
operator!=(const Row<N,E1,S1>& l, const Row<N,E2,S2>& r) {return !(l==r);} 


////////////////////////////////////////////////////
// Global operators involving a row and a scalar. //
////////////////////////////////////////////////////

// I haven't been able to figure out a nice way to templatize for the
// built-in reals without introducing a lot of unwanted type matches
// as well. So we'll just grind them out explicitly here.

// SCALAR MULTIPLY

// v = v*real, real*v 
template <int N, class E, int S> inline
typename Row<N,E,S>::template Result<float>::Mul
operator*(const Row<N,E,S>& l, const float& r)
  { return Row<N,E,S>::template Result<float>::MulOp::perform(l,r); }
template <int N, class E, int S> inline
typename Row<N,E,S>::template Result<float>::Mul
operator*(const float& l, const Row<N,E,S>& r) {return r*l;}

template <int N, class E, int S> inline
typename Row<N,E,S>::template Result<double>::Mul
operator*(const Row<N,E,S>& l, const double& r)
  { return Row<N,E,S>::template Result<double>::MulOp::perform(l,r); }
template <int N, class E, int S> inline
typename Row<N,E,S>::template Result<double>::Mul
operator*(const double& l, const Row<N,E,S>& r) {return r*l;}

template <int N, class E, int S> inline
typename Row<N,E,S>::template Result<long double>::Mul
operator*(const Row<N,E,S>& l, const long double& r)
  { return Row<N,E,S>::template Result<long double>::MulOp::perform(l,r); }
template <int N, class E, int S> inline
typename Row<N,E,S>::template Result<long double>::Mul
operator*(const long double& l, const Row<N,E,S>& r) {return r*l;}

// v = v*int, int*v -- just convert int to v's precision float
template <int N, class E, int S> inline
typename Row<N,E,S>::template Result<typename CNT<E>::Precision>::Mul
operator*(const Row<N,E,S>& l, int r) {return l * (typename CNT<E>::Precision)r;}
template <int N, class E, int S> inline
typename Row<N,E,S>::template Result<typename CNT<E>::Precision>::Mul
operator*(int l, const Row<N,E,S>& r) {return r * (typename CNT<E>::Precision)l;}

// Complex, conjugate, and negator are all easy to templatize.

// v = v*complex, complex*v
template <int N, class E, int S, class R> inline
typename Row<N,E,S>::template Result<std::complex<R> >::Mul
operator*(const Row<N,E,S>& l, const std::complex<R>& r)
  { return Row<N,E,S>::template Result<std::complex<R> >::MulOp::perform(l,r); }
template <int N, class E, int S, class R> inline
typename Row<N,E,S>::template Result<std::complex<R> >::Mul
operator*(const std::complex<R>& l, const Row<N,E,S>& r) {return r*l;}

// v = v*conjugate, conjugate*v (convert conjugate->complex)
template <int N, class E, int S, class R> inline
typename Row<N,E,S>::template Result<std::complex<R> >::Mul
operator*(const Row<N,E,S>& l, const conjugate<R>& r) {return l*(std::complex<R>)r;}
template <int N, class E, int S, class R> inline
typename Row<N,E,S>::template Result<std::complex<R> >::Mul
operator*(const conjugate<R>& l, const Row<N,E,S>& r) {return r*(std::complex<R>)l;}

// v = v*negator, negator*v: convert negator to standard number
template <int N, class E, int S, class R> inline
typename Row<N,E,S>::template Result<typename negator<R>::StdNumber>::Mul
operator*(const Row<N,E,S>& l, const negator<R>& r) {return l * (typename negator<R>::StdNumber)(R)r;}
template <int N, class E, int S, class R> inline
typename Row<N,E,S>::template Result<typename negator<R>::StdNumber>::Mul
operator*(const negator<R>& l, const Row<N,E,S>& r) {return r * (typename negator<R>::StdNumber)(R)l;}


// SCALAR DIVIDE. This is a scalar operation when the scalar is on the right,
// but when it is on the left it means scalar * pseudoInverse(row), which is a vec.

// v = v/real, real/v 
template <int N, class E, int S> inline
typename Row<N,E,S>::template Result<float>::Dvd
operator/(const Row<N,E,S>& l, const float& r)
  { return Row<N,E,S>::template Result<float>::DvdOp::perform(l,r); }
template <int N, class E, int S> inline
typename CNT<float>::template Result<Row<N,E,S> >::Dvd
operator/(const float& l, const Row<N,E,S>& r)
  { return CNT<float>::template Result<Row<N,E,S> >::DvdOp::perform(l,r); }

template <int N, class E, int S> inline
typename Row<N,E,S>::template Result<double>::Dvd
operator/(const Row<N,E,S>& l, const double& r)
  { return Row<N,E,S>::template Result<double>::DvdOp::perform(l,r); }
template <int N, class E, int S> inline
typename CNT<double>::template Result<Row<N,E,S> >::Dvd
operator/(const double& l, const Row<N,E,S>& r)
  { return CNT<double>::template Result<Row<N,E,S> >::DvdOp::perform(l,r); }

template <int N, class E, int S> inline
typename Row<N,E,S>::template Result<long double>::Dvd
operator/(const Row<N,E,S>& l, const long double& r)
  { return Row<N,E,S>::template Result<long double>::DvdOp::perform(l,r); }
template <int N, class E, int S> inline
typename CNT<long double>::template Result<Row<N,E,S> >::Dvd
operator/(const long double& l, const Row<N,E,S>& r)
  { return CNT<long double>::template Result<Row<N,E,S> >::DvdOp::perform(l,r); }

// v = v/int, int/v -- just convert int to v's precision float
template <int N, class E, int S> inline
typename Row<N,E,S>::template Result<typename CNT<E>::Precision>::Dvd
operator/(const Row<N,E,S>& l, int r) {return l / (typename CNT<E>::Precision)r;}
template <int N, class E, int S> inline
typename CNT<typename CNT<E>::Precision>::template Result<Row<N,E,S> >::Dvd
operator/(int l, const Row<N,E,S>& r) {return (typename CNT<E>::Precision)l / r;}


// Complex, conjugate, and negator are all easy to templatize.

// v = v/complex, complex/v
template <int N, class E, int S, class R> inline
typename Row<N,E,S>::template Result<std::complex<R> >::Dvd
operator/(const Row<N,E,S>& l, const std::complex<R>& r)
  { return Row<N,E,S>::template Result<std::complex<R> >::DvdOp::perform(l,r); }
template <int N, class E, int S, class R> inline
typename CNT<std::complex<R> >::template Result<Row<N,E,S> >::Dvd
operator/(const std::complex<R>& l, const Row<N,E,S>& r)
  { return CNT<std::complex<R> >::template Result<Row<N,E,S> >::DvdOp::perform(l,r); }

// v = v/conjugate, conjugate/v (convert conjugate->complex)
template <int N, class E, int S, class R> inline
typename Row<N,E,S>::template Result<std::complex<R> >::Dvd
operator/(const Row<N,E,S>& l, const conjugate<R>& r) {return l/(std::complex<R>)r;}
template <int N, class E, int S, class R> inline
typename CNT<std::complex<R> >::template Result<Row<N,E,S> >::Dvd
operator/(const conjugate<R>& l, const Row<N,E,S>& r) {return (std::complex<R>)l/r;}

// v = v/negator, negator/v: convert negator to number
template <int N, class E, int S, class R> inline
typename Row<N,E,S>::template Result<typename negator<R>::StdNumber>::Dvd
operator/(const Row<N,E,S>& l, const negator<R>& r) {return l/(typename negator<R>::StdNumber)(R)r;}
template <int N, class E, int S, class R> inline
typename CNT<R>::template Result<Row<N,E,S> >::Dvd
operator/(const negator<R>& l, const Row<N,E,S>& r) {return (typename negator<R>::StdNumber)(R)l/r;}


// Add and subtract are odd as scalar ops. They behave as though the
// scalar stands for a vector each of whose elements is that scalar,
// and then a normal vector add or subtract is done.

// SCALAR ADD

// v = v+real, real+v 
template <int N, class E, int S> inline
typename Row<N,E,S>::template Result<float>::Add
operator+(const Row<N,E,S>& l, const float& r)
  { return Row<N,E,S>::template Result<float>::AddOp::perform(l,r); }
template <int N, class E, int S> inline
typename Row<N,E,S>::template Result<float>::Add
operator+(const float& l, const Row<N,E,S>& r) {return r+l;}

template <int N, class E, int S> inline
typename Row<N,E,S>::template Result<double>::Add
operator+(const Row<N,E,S>& l, const double& r)
  { return Row<N,E,S>::template Result<double>::AddOp::perform(l,r); }
template <int N, class E, int S> inline
typename Row<N,E,S>::template Result<double>::Add
operator+(const double& l, const Row<N,E,S>& r) {return r+l;}

template <int N, class E, int S> inline
typename Row<N,E,S>::template Result<long double>::Add
operator+(const Row<N,E,S>& l, const long double& r)
  { return Row<N,E,S>::template Result<long double>::AddOp::perform(l,r); }
template <int N, class E, int S> inline
typename Row<N,E,S>::template Result<long double>::Add
operator+(const long double& l, const Row<N,E,S>& r) {return r+l;}

// v = v+int, int+v -- just convert int to v's precision float
template <int N, class E, int S> inline
typename Row<N,E,S>::template Result<typename CNT<E>::Precision>::Add
operator+(const Row<N,E,S>& l, int r) {return l + (typename CNT<E>::Precision)r;}
template <int N, class E, int S> inline
typename Row<N,E,S>::template Result<typename CNT<E>::Precision>::Add
operator+(int l, const Row<N,E,S>& r) {return r + (typename CNT<E>::Precision)l;}

// Complex, conjugate, and negator are all easy to templatize.

// v = v+complex, complex+v
template <int N, class E, int S, class R> inline
typename Row<N,E,S>::template Result<std::complex<R> >::Add
operator+(const Row<N,E,S>& l, const std::complex<R>& r)
  { return Row<N,E,S>::template Result<std::complex<R> >::AddOp::perform(l,r); }
template <int N, class E, int S, class R> inline
typename Row<N,E,S>::template Result<std::complex<R> >::Add
operator+(const std::complex<R>& l, const Row<N,E,S>& r) {return r+l;}

// v = v+conjugate, conjugate+v (convert conjugate->complex)
template <int N, class E, int S, class R> inline
typename Row<N,E,S>::template Result<std::complex<R> >::Add
operator+(const Row<N,E,S>& l, const conjugate<R>& r) {return l+(std::complex<R>)r;}
template <int N, class E, int S, class R> inline
typename Row<N,E,S>::template Result<std::complex<R> >::Add
operator+(const conjugate<R>& l, const Row<N,E,S>& r) {return r+(std::complex<R>)l;}

// v = v+negator, negator+v: convert negator to standard number
template <int N, class E, int S, class R> inline
typename Row<N,E,S>::template Result<typename negator<R>::StdNumber>::Add
operator+(const Row<N,E,S>& l, const negator<R>& r) {return l + (typename negator<R>::StdNumber)(R)r;}
template <int N, class E, int S, class R> inline
typename Row<N,E,S>::template Result<typename negator<R>::StdNumber>::Add
operator+(const negator<R>& l, const Row<N,E,S>& r) {return r + (typename negator<R>::StdNumber)(R)l;}

// SCALAR SUBTRACT -- careful, not commutative.

// v = v-real, real-v 
template <int N, class E, int S> inline
typename Row<N,E,S>::template Result<float>::Sub
operator-(const Row<N,E,S>& l, const float& r)
  { return Row<N,E,S>::template Result<float>::SubOp::perform(l,r); }
template <int N, class E, int S> inline
typename CNT<float>::template Result<Row<N,E,S> >::Sub
operator-(const float& l, const Row<N,E,S>& r)
  { return CNT<float>::template Result<Row<N,E,S> >::SubOp::perform(l,r); }

template <int N, class E, int S> inline
typename Row<N,E,S>::template Result<double>::Sub
operator-(const Row<N,E,S>& l, const double& r)
  { return Row<N,E,S>::template Result<double>::SubOp::perform(l,r); }
template <int N, class E, int S> inline
typename CNT<double>::template Result<Row<N,E,S> >::Sub
operator-(const double& l, const Row<N,E,S>& r)
  { return CNT<double>::template Result<Row<N,E,S> >::SubOp::perform(l,r); }

template <int N, class E, int S> inline
typename Row<N,E,S>::template Result<long double>::Sub
operator-(const Row<N,E,S>& l, const long double& r)
  { return Row<N,E,S>::template Result<long double>::SubOp::perform(l,r); }
template <int N, class E, int S> inline
typename CNT<long double>::template Result<Row<N,E,S> >::Sub
operator-(const long double& l, const Row<N,E,S>& r)
  { return CNT<long double>::template Result<Row<N,E,S> >::SubOp::perform(l,r); }

// v = v-int, int-v // just convert int to v's precision float
template <int N, class E, int S> inline
typename Row<N,E,S>::template Result<typename CNT<E>::Precision>::Sub
operator-(const Row<N,E,S>& l, int r) {return l - (typename CNT<E>::Precision)r;}
template <int N, class E, int S> inline
typename CNT<typename CNT<E>::Precision>::template Result<Row<N,E,S> >::Sub
operator-(int l, const Row<N,E,S>& r) {return (typename CNT<E>::Precision)l - r;}


// Complex, conjugate, and negator are all easy to templatize.

// v = v-complex, complex-v
template <int N, class E, int S, class R> inline
typename Row<N,E,S>::template Result<std::complex<R> >::Sub
operator-(const Row<N,E,S>& l, const std::complex<R>& r)
  { return Row<N,E,S>::template Result<std::complex<R> >::SubOp::perform(l,r); }
template <int N, class E, int S, class R> inline
typename CNT<std::complex<R> >::template Result<Row<N,E,S> >::Sub
operator-(const std::complex<R>& l, const Row<N,E,S>& r)
  { return CNT<std::complex<R> >::template Result<Row<N,E,S> >::SubOp::perform(l,r); }

// v = v-conjugate, conjugate-v (convert conjugate->complex)
template <int N, class E, int S, class R> inline
typename Row<N,E,S>::template Result<std::complex<R> >::Sub
operator-(const Row<N,E,S>& l, const conjugate<R>& r) {return l-(std::complex<R>)r;}
template <int N, class E, int S, class R> inline
typename CNT<std::complex<R> >::template Result<Row<N,E,S> >::Sub
operator-(const conjugate<R>& l, const Row<N,E,S>& r) {return (std::complex<R>)l-r;}

// v = v-negator, negator-v: convert negator to standard number
template <int N, class E, int S, class R> inline
typename Row<N,E,S>::template Result<typename negator<R>::StdNumber>::Sub
operator-(const Row<N,E,S>& l, const negator<R>& r) {return l-(typename negator<R>::StdNumber)(R)r;}
template <int N, class E, int S, class R> inline
typename CNT<R>::template Result<Row<N,E,S> >::Sub
operator-(const negator<R>& l, const Row<N,E,S>& r) {return (typename negator<R>::StdNumber)(R)l-r;}


// Row I/O
template <int N, class E, int S, class CHAR, class TRAITS> inline
std::basic_ostream<CHAR,TRAITS>&
operator<<(std::basic_ostream<CHAR,TRAITS>& o, const Row<N,E,S>& v) {
    o << "[" << v[0]; for(int i=1;i<N;++i) o<<','<<v[i]; o<<']'; return o;
}

template <int N, class E, int S, class CHAR, class TRAITS> inline
std::basic_istream<CHAR,TRAITS>&
operator>>(std::basic_istream<CHAR,TRAITS>& is, Row<N,E,S>& v) {
    // TODO: not sure how to do Row input yet
    assert(false);
    return is;
}

} //namespace SimTK


#endif //SimTK_SIMMATRIX_SMALLMATRIX_ROW_H_
