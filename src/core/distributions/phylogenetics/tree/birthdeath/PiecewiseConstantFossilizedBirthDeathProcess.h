#ifndef PiecewiseConstantFossilizedBirthDeathProcess_H
#define PiecewiseConstantFossilizedBirthDeathProcess_H

#include "RbVector.h"
#include "AbstractBirthDeathProcess.h"

namespace RevBayesCore {
    
    class Clade;
    class Taxon;
    
    /**
     * @brief Piecewise-constant fossilized birth-death process with serially sampled fossils.
     *
     * The piecewise-constant birth-death process has constant rates for each time interval.
     * At the end of each time interval there may be an abrupt rate-shift (jump) for each
     * of the rates. Additionally, there may be sampling at the end of each interval.
     * Finally, fossils are sampled with rate psi, the others (fossils and extant taxa) are
     * sampled at sampling times (including the present).
     *
     * We assume that the rate vectors have one more element than the rate-change vectors.
     * Thus, one rate-change means always two interval, two rate-changes three interval, and so on.
     *
     *
     * @copyright Copyright 2009-
     * @author The RevBayes Development Core Team (Walker Pett)
     * @since 2014-03-18, version 1.0
     *
     */
    class PiecewiseConstantFossilizedBirthDeathProcess : public AbstractBirthDeathProcess {
        
    public:
        PiecewiseConstantFossilizedBirthDeathProcess (const TypedDagNode<double>* ra,
                                                      const DagNode *speciation,
                                                      const DagNode *extinction,
                                                      const DagNode *psi,
                                                      const DagNode *counts,
                                                      const TypedDagNode<double>* rho,
                                                      const TypedDagNode<RbVector<double> > *times,
                                                      const std::string &condition,
                                                      const std::vector<Taxon> &taxa,
                                                      bool uo,
                                                      bool pa );  //!< Constructor
        
        // public member functions
        PiecewiseConstantFossilizedBirthDeathProcess*   clone(void) const;                                         //!< Create an independent clone

        double                                          getExtinctionRate( size_t index ) const;
        long                                            getFossilCount( size_t index, size_t taxon ) const;
        long                                            getFossilCount( size_t index ) const;
        double                                          getFossilizationRate( size_t index ) const;
        double                                          getIntervalTime( size_t index ) const;
        double                                          getSpeciationRate( size_t index ) const;

        void                                            simulateClade(std::vector<TopologyNode *> &n, double age, double present);

    protected:
        // Parameter management functions
        double                                          computeLnProbabilityTimes(void) const;                            //!< Compute the log-transformed probability of the current value.

        double                                          lnProbNumTaxa(size_t n, double start, double end, bool MRCA) const { throw RbException("Cannot compute P(nTaxa)."); }
        double                                          lnProbTreeShape(void) const;

        double                                          simulateDivergenceTime(double origin, double present) const;    //!< Simulate a speciation event.

        // Parameter management functions
        void                                            swapParameterInternal(const DagNode *oldP, const DagNode *newP);                //!< Swap a parameter

        void                                            keepSpecialization(DagNode *toucher);
        void                                            restoreSpecialization(DagNode *toucher);
        void                                            touchSpecialization(DagNode *toucher, bool touchAll);

    private:
        
        // helper functions
        double                                          getBirthTime( const TopologyNode& ) const;
        double                                          getMaxTaxonAge( const TopologyNode& ) const;
        size_t                                          l(double t) const;                                     //!< Find the index so that times[index-1] < t < times[index]
        double                                          pSurvival(double start, double end) const;             //!< Compute the probability of survival of the process (without incomplete taxon sampling).
        double                                          p(size_t i, double t) const;
        double                                          q(size_t i, double t, bool tilde = false) const;
        double                                          integrateQ(size_t i, double t) const;

        void                                            updateIntervals() const;


        bool                                            ascending;

        size_t                                          num_intervals;

        // members
        const TypedDagNode<double >*                    homogeneous_lambda;                                    //!< The homogeneous speciation rates.
        const TypedDagNode<RbVector<double> >*          heterogeneous_lambda;                                  //!< The heterogeneous speciation rates.
        const TypedDagNode<double >*                    homogeneous_mu;                                        //!< The homogeneous speciation rates.
        const TypedDagNode<RbVector<double> >*          heterogeneous_mu;                                      //!< The heterogeneous speciation rates.
        const TypedDagNode<double >*                    homogeneous_psi;                                       //!< The homogeneous speciation rates.
        const TypedDagNode<RbVector<double> >*          heterogeneous_psi;                                     //!< The heterogeneous speciation rates.
        const TypedDagNode<double >*                    homogeneous_rho;                                       //!< The homogeneous speciation rates.
        const TypedDagNode<RbVector<double> >*          timeline;                                              //!< The times of the instantaneous sampling events.
        const TypedDagNode<long>*                       fossil_counts;                                         //!< The number of fossil observations, per interval.
        const TypedDagNode<RbVector<long> >*            interval_fossil_counts;                                //!< The number of fossil observations, per interval.
        const TypedDagNode<RbVector<RbVector<long> > >* species_interval_fossil_counts;                        //!< The number of fossil observations, per species/interval.

        mutable std::vector<double>                     birth;
        mutable std::vector<double>                     death;
        mutable std::vector<double>                     fossil;
        mutable std::vector<double>                     times;

        mutable std::vector<double>                     q_i;
        mutable std::vector<double>                     q_tilde_i;
        mutable std::vector<double>                     p_i;

        std::string                                     condition;
        std::vector<Taxon>                              taxa;                                                                                               //!< Taxon names that will be attached to new simulated trees.

        bool                                            marginalize_k;
        bool                                            presence_absence;

        mutable std::vector<size_t>                     oldest_intervals;
        mutable std::vector<size_t>                     youngest_intervals;
    };
}

#endif
