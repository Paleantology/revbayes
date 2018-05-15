#ifndef CholeskyDecomposition_H
#define CholeskyDecomposition_H

#include "MatrixComplex.h"
#include "MatrixReal.h"

#include <complex>


namespace RevBayesCore {

    class CholeskyDecomposition {

    public:
                                                CholeskyDecomposition(const MatrixReal* m);

        void                                    update(void);
        const MatrixReal                        getInverse(void) const { return inverseMatrix; }
        double                                  computeLogDet(void);
        const MatrixReal                        getLowerCholeskyFactor(void) const { return L; }
//        MatrixReal                              getLowerCholeskyFactor(void) { return L; }
        const bool                              checkPositiveSemidefinite(void) const { return is_positive_semidefinite; }

    private:

        void                                    computeInverse(void);
        void                                    decomposeMatrix(void);
        
        size_t                                  n;                                              //!< Row and column dimension (square matrix)
        const MatrixReal*                       qPtr;                                           //!< A pointer to the matrix for this cholesky decomposition
        MatrixReal                              L;
        MatrixReal                              inverseMatrix;
        bool                                    is_positive_semidefinite;

    };
    
}

#endif
