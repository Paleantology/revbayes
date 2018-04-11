#include "DistributionExponential.h"
#include "PiecewiseConstantFossilizedBirthDeathRangeProcess.h"
#include "RandomNumberFactory.h"
#include "RandomNumberGenerator.h"
#include "RbConstants.h"
#include "RbMathLogic.h"
#include "StochasticNode.h"
#include "TypedDistribution.h"

#include <algorithm>
#include <cmath>

using namespace RevBayesCore;

/**
 * Constructor. 
 * We delegate most parameters to the base class and initialize the members.
 *
 * \param[in]    s              Speciation rates.
 * \param[in]    e              Extinction rates.
 * \param[in]    p              Fossil sampling rates.
 * \param[in]    c              Fossil observation counts.
 * \param[in]    r              Instantaneous sampling probabilities.
 * \param[in]    t              Rate change times.
 * \param[in]    cdt            Condition of the process (none/survival/#Taxa).
 * \param[in]    tn             Taxa.
 */
PiecewiseConstantFossilizedBirthDeathRangeProcess::PiecewiseConstantFossilizedBirthDeathRangeProcess(const DagNode *inspeciation,
                                                                                                     const DagNode *inextinction,
                                                                                                     const DagNode *inpsi,
                                                                                                     const DagNode *incounts,
                                                                                                     const TypedDagNode<double> *inrho,
                                                                                                     const TypedDagNode< RbVector<double> > *intimes,
                                                                                                     const std::string &incondition,
                                                                                                     const std::vector<Taxon> &intaxa,
                                                                                                     bool pa ) : TypedDistribution<MatrixReal>(new MatrixReal(intaxa.size(), 2)),
    ascending(false), homogeneous_rho(inrho), timeline( intimes ), condition(incondition), taxa(intaxa), presence_absence(pa)
{
    // initialize all the pointers to NULL
    homogeneous_lambda             = NULL;
    homogeneous_mu                 = NULL;
    homogeneous_psi                = NULL;
    fossil_counts                  = NULL;
    heterogeneous_lambda           = NULL;
    heterogeneous_mu               = NULL;
    heterogeneous_psi              = NULL;
    interval_fossil_counts         = NULL;
    species_interval_fossil_counts = NULL;

    RbException no_timeline_err = RbException("No time intervals provided for piecewise constant fossilized birth death process");

    heterogeneous_lambda = dynamic_cast<const TypedDagNode<RbVector<double> >*>(inspeciation);
    homogeneous_lambda = dynamic_cast<const TypedDagNode<double >*>(inspeciation);

    addParameter( homogeneous_lambda );
    addParameter( heterogeneous_lambda );

    if ( heterogeneous_lambda == NULL && homogeneous_lambda == NULL)
    {
        throw(RbException("Speciation rate must be of type RealPos or RealPos[]"));
    }
    else if( heterogeneous_lambda != NULL )
    {
        if( timeline == NULL ) throw(no_timeline_err);

        if (heterogeneous_lambda->getValue().size() != timeline->getValue().size() + 1)
        {
            std::stringstream ss;
            ss << "Number of speciation rates (" << heterogeneous_lambda->getValue().size() << ") does not match number of time intervals (" << timeline->getValue().size() + 1 << ")";
            throw(RbException(ss.str()));
        }
    }


    heterogeneous_mu = dynamic_cast<const TypedDagNode<RbVector<double> >*>(inextinction);
    homogeneous_mu = dynamic_cast<const TypedDagNode<double >*>(inextinction);

    addParameter( homogeneous_mu );
    addParameter( heterogeneous_mu );

    if ( heterogeneous_mu == NULL && homogeneous_mu == NULL)
    {
        throw(RbException("Extinction rate must be of type RealPos or RealPos[]"));
    }
    else if( heterogeneous_mu != NULL )
    {
        if( timeline == NULL ) throw(no_timeline_err);

        if (heterogeneous_mu->getValue().size() != timeline->getValue().size() + 1)
        {
            std::stringstream ss;
            ss << "Number of extinction rates (" << heterogeneous_mu->getValue().size() << ") does not match number of time intervals (" << timeline->getValue().size() + 1 << ")";
            throw(RbException(ss.str()));
        }
    }


    heterogeneous_psi = dynamic_cast<const TypedDagNode<RbVector<double> >*>(inpsi);
    homogeneous_psi = dynamic_cast<const TypedDagNode<double >*>(inpsi);

    addParameter( homogeneous_psi );
    addParameter( heterogeneous_psi );

    if ( heterogeneous_psi == NULL && homogeneous_psi == NULL)
    {
        throw(RbException("Fossilization rate must be of type RealPos or RealPos[]"));
    }
    else if( heterogeneous_psi != NULL )
    {
        if( timeline == NULL ) throw(no_timeline_err);

        if (heterogeneous_psi->getValue().size() != timeline->getValue().size() + 1)
        {
            std::stringstream ss;
            ss << "Number of fossilization rates (" << heterogeneous_psi->getValue().size() << ") does not match number of time intervals (" << timeline->getValue().size() + 1 << ")";
            throw(RbException(ss.str()));
        }
    }

    species_interval_fossil_counts = dynamic_cast<const TypedDagNode<RbVector<RbVector<long> > >*>(incounts);
    interval_fossil_counts         = dynamic_cast<const TypedDagNode<RbVector<long> >*>(incounts);
    fossil_counts                  = dynamic_cast<const TypedDagNode<long> *>(incounts);

    addParameter( species_interval_fossil_counts );
    addParameter( interval_fossil_counts );
    addParameter( fossil_counts );

    marginalize_k = ( species_interval_fossil_counts == NULL && interval_fossil_counts == NULL && fossil_counts == NULL);

    if( marginalize_k && presence_absence )
    {
        throw(RbException("Cannot marginalize fossil presence absence data"));
    }

    if( species_interval_fossil_counts == NULL && presence_absence )
    {
        throw(RbException("Presence absence data must be provided by species and interval"));
    }

    if ( fossil_counts != NULL && homogeneous_psi == NULL)
    {
        throw(RbException("Heterogeneous fossil sampling rates provided, but homogeneous fossil counts"));
    }
    else if ( interval_fossil_counts != NULL || species_interval_fossil_counts != NULL )
    {
        if( timeline == NULL ) throw(no_timeline_err);

        if ( interval_fossil_counts != NULL && interval_fossil_counts->getValue().size() != timeline->getValue().size() + 1)
        {
            std::stringstream ss;
            ss << "Number of fossil counts (" << interval_fossil_counts->getValue().size() << ") does not match number of time intervals (" << timeline->getValue().size() + 1 << ")";
            throw(RbException(ss.str()));
        }
        else if ( species_interval_fossil_counts != NULL && species_interval_fossil_counts->getValue().size() != taxa.size())
        {
            std::stringstream ss;
            ss << "Number of species fossil counts (" << species_interval_fossil_counts->getValue().size() << ") does not match number of taxa (" << taxa.size() << ")";
            throw(RbException(ss.str()));
        }
        else if ( species_interval_fossil_counts != NULL && species_interval_fossil_counts->getValue().front().size() != timeline->getValue().size() + 1)
        {
            std::stringstream ss;
            ss << "Number of fossil counts per species (" << species_interval_fossil_counts->getValue().front().size() << ") does not match number of time intervals (" << timeline->getValue().size() + 1 << ")";
            throw(RbException(ss.str()));
        }
    }

    addParameter( homogeneous_rho );
    addParameter( timeline );
    
    num_intervals = timeline == NULL ? 1 : timeline->getValue().size()+1;

    if ( num_intervals > 1 )
    {
        std::vector<double> times = timeline->getValue();
        std::vector<double> times_sorted_ascending = times;
        std::vector<double> times_sorted_descending = times;

        sort(times_sorted_ascending.begin(), times_sorted_ascending.end() );
        sort(times_sorted_descending.rbegin(), times_sorted_descending.rend() );

        if( times == times_sorted_ascending )
        {
            ascending = true;
        }
        else if ( times != times_sorted_ascending )
        {
            throw(RbException("Interval times must be provided in order"));
        }
    }

    p_i         = std::vector<double>(num_intervals+1, 1.0);
    q_i         = std::vector<double>(num_intervals+1, 1.0);
    q_tilde_i   = std::vector<double>(num_intervals+1, 1.0);

    birth       = std::vector<double>(num_intervals, 0.0);
    death       = std::vector<double>(num_intervals, 0.0);
    fossil      = std::vector<double>(num_intervals, 0.0);
    times       = std::vector<double>(num_intervals, 0.0);

    dirty_gamma = std::vector<bool>(taxa.size(), true);
    gamma_i     = std::vector<size_t>(taxa.size(), 0);
    gamma_links = std::vector<std::vector<bool> >(taxa.size(), std::vector<bool>(taxa.size(), false));

    oldest_intervals = std::vector<size_t>( taxa.size(), num_intervals - 1 );
    youngest_intervals = std::vector<size_t>( taxa.size(), num_intervals - 1 );

    redrawValue();
    updateGamma(true);
}


/**
 * The clone function is a convenience function to create proper copies of inherited objected.
 * E.g. a.clone() will create a clone of the correct type even if 'a' is of derived type 'B'.
 *
 * \return A new copy of myself 
 */
PiecewiseConstantFossilizedBirthDeathRangeProcess* PiecewiseConstantFossilizedBirthDeathRangeProcess::clone( void ) const
{
    return new PiecewiseConstantFossilizedBirthDeathRangeProcess( *this );
}


/**
 * Compute the log-transformed probability of the current value under the current parameter values.
 *
 */
double PiecewiseConstantFossilizedBirthDeathRangeProcess::computeLnProbability( void )
{
    // prepare the probability computation
    updateIntervals();
    updateGamma();

    // variable declarations and initialization
    double lnProbTimes = 0.0;
    
    size_t num_extant_sampled = 0;
    size_t num_extant_unsampled = 0;

    double maxb = 0;
    double maxl = 0;

    std::vector<long> kappa_prime (num_intervals, 0);
    std::vector<double> L (num_intervals, 0.0);

    // add the fossil tip age terms
    for (size_t i = 0; i < taxa.size(); ++i)
    {
        if ( RbMath::isFinite(lnProbTimes) == false )
        {
            return RbConstants::Double::neginf;
        }
        

        double b = (*this->value)[i][0];
        double d = (*this->value)[i][1];
        double o = taxa[i].getAgeRange().getMax();
        double y = taxa[i].getAgeRange().getMin();

        size_t bi = l(b);
        size_t di = l(d);
        size_t oi = presence_absence ? oldest_intervals[i] : l(o);
        size_t yi = presence_absence ? youngest_intervals[i] : l(y);


        // check constraints
        if( presence_absence )
        {
            if( !( b > d && (( y == 0.0 && d == 0.0 ) || ( y != 0.0 && d >= 0.0 && yi <= di )) ) )
            {
                return RbConstants::Double::neginf;
            }
        }
        else if ( !( b > o && o >= y && (y > d || (y == d && y == 0.0)) && d >= 0.0 ) )
        {
            return RbConstants::Double::neginf;
        }


        // count the number of rho-sampled tips
        num_extant_sampled  += (d == 0.0 && y == 0.0);  // l
        num_extant_unsampled += (d == 0.0 && y != 0.0); // n - m - l


        // find the origin time
        if (b > maxb)
        {
            maxb = b;
            maxl = birth[bi];
        }


        // include speciation density
        lnProbTimes += log( birth[bi] );

        // multiply by the number of possible birth locations
        lnProbTimes += log( gamma_i[i] == 0 ? 1 : gamma_i[i] );

        // multiply by q at the birth time
        lnProbTimes += log( q(bi, b) );

        // include intermediate q terms
        for (size_t j = bi; j < oi; j++)
        {
            lnProbTimes += log( q_i[j+1] );
        }

        // include factor for the first appearance
        if( presence_absence == false )
        {
            lnProbTimes += log( q(oi, o, true) ) - log( q(oi, o) );
        }

        // include intermediate q_tilde terms
        for (size_t j = oi; j < di; j++)
        {
            lnProbTimes += log( q_tilde_i[j+1] );
        }

        // divide by q_tilde at the death time
        lnProbTimes -= log( q( di, d, true) );

        // include extinction density
        if (d > 0.0) lnProbTimes += log( death[di] );


        // update the marginalized fossil count data
        if( marginalize_k )
        {
            if( o > 0.0 )
            {
                kappa_prime[oi]++;
            }
            if( o != y && y > 0.0)
            {
                kappa_prime[yi]++;
            }

            if( oi == yi )
            {
                L[oi] += o - y;
            }
            else
            {
                L[oi] += o - times[oi];

                for(size_t j = oi + 1; j < yi; j++)
                {
                    L[j] += times[j-1] - times[j];
                }

                L[yi] += times[yi-1] - y;
            }
        }
        else if( presence_absence )
        {
            if( bi == di )
            {
                long count = getFossilCount(bi,i);

                if( count > 0 )
                {
                    L[bi] += log( integrateQ(bi,b) - integrateQ(di,d) ) + log(fossil[bi]) - fossil[di]*( d-getIntervalTime(di) );
                }
            }
            else
            {
                bool first = true;

                long count = getFossilCount(bi,i);

                double Ls = b - times[bi];
                if( count > 0 )
                {
                    L[bi] += log( integrateQ( bi, b ) - integrateQ( bi, times[bi] ) ) + log(fossil[bi]);
                    first = false;
                }

                for(size_t j = bi + 1; j < di; j++)
                {
                    count = getFossilCount(j,i);
                    if( count > 0 )
                    {
                        if( first )
                        {
                            L[j] += log( integrateQ( j, times[j-1] ) - integrateQ( j, times[j] ) ) + log(fossil[j]);
                            first = false;
                        }
                        else
                        {
                            Ls = times[j-1] - times[j];
                            L[j] += fossil[j]*Ls + log( 1.0 - exp( - Ls * fossil[j] ) );
                        }
                    }
                }

                count = getFossilCount(di,i);
                if( count > 0 )
                {
                    if( first )
                    {
                        L[di] += log( integrateQ( di, times[di-1] ) - integrateQ( di, d ) ) + log(fossil[di]) - fossil[di]*( d-times[di] );
                        first = false;
                    }
                    else
                    {
                        Ls = times[di-1] - d;
                        L[di] += fossil[di]*Ls + log( 1.0 - exp( - Ls * fossil[di] ) );
                    }
                }
            }
        }
    }
    
    // the origin is not a speciation event
    lnProbTimes -= log(maxl);

    // check if we marginalize over fossil counts
    for (size_t i = 0; i < num_intervals; ++i)
    {
        if( presence_absence )
        {
            lnProbTimes += L[i];
        }
        else
        {
            size_t k;

            if( marginalize_k )
            {
                k = kappa_prime[i];

                lnProbTimes += fossil[i]*L[i];
            }
            else
            {
                k = getFossilCount(i);
            }

            lnProbTimes += k*log(fossil[i]);
        }
    }

    // add the sampled extant tip age term
    if ( homogeneous_rho->getValue() > 0.0)
        lnProbTimes += num_extant_sampled * log( homogeneous_rho->getValue() );
    // add the unsampled extant tip age term
    if ( homogeneous_rho->getValue() < 1.0)
        lnProbTimes += num_extant_unsampled * log( 1.0 - homogeneous_rho->getValue() );

    

    // condition on survival
    if ( condition == "survival" )
    {
        lnProbTimes -= log( pSurvival(maxb,0) );
    }

    if ( RbMath::isFinite(lnProbTimes) == false )
    {
        return RbConstants::Double::neginf;
    }


    return lnProbTimes;
}


/**
 * Compute the number of ranges that intersect with range i
 *
 * \param[in]    i      index of range for which to compute gamma
 *
 * \return Small gamma
 */
void PiecewiseConstantFossilizedBirthDeathRangeProcess::updateGamma(bool force)
{
    for (size_t i = 0; i < taxa.size(); i++)
    {
        if ( dirty_gamma[i] || force )
        {
            double ai = (*this->value)[i][0];
            double bi = (*this->value)[i][1];

            if ( force == true ) gamma_i[i] = 0;

            for (size_t j = 0; j < taxa.size(); j++)
            {
                if (i == j) continue;

                double aj = (*this->value)[j][0];
                double bj = (*this->value)[j][1];

                bool linki = ( ai < aj && ai > bj );
                bool linkj = ( aj < ai && aj > bi );

                if ( gamma_links[i][j] != linki && force == false )
                {
                    gamma_i[i] += linki ? 1 : -1;
                }
                if ( gamma_links[j][i] != linkj && force == false )
                {
                    gamma_i[j] += linkj ? 1 : -1;
                }

                if ( force == true ) gamma_i[i] += linki;

                gamma_links[i][j] = linki;
                gamma_links[j][i] = linkj;
            }
        }
    }
}


double PiecewiseConstantFossilizedBirthDeathRangeProcess::getExtinctionRate( size_t index ) const
{

    // remove the old parameter first
    if ( homogeneous_mu != NULL )
    {
        return homogeneous_mu->getValue();
    }
    else
    {
        size_t num = heterogeneous_mu->getValue().size();

        if (index >= num)
        {
            throw(RbException("Extinction rate index out of bounds"));
        }
        return ascending ? heterogeneous_mu->getValue()[num - 1 - index] : heterogeneous_mu->getValue()[index];
    }
}


long PiecewiseConstantFossilizedBirthDeathRangeProcess::getFossilCount( size_t interval, size_t species ) const
{

    // remove the old parameter first
    if ( fossil_counts != NULL )
    {
        return (int)fossil_counts->getValue();
    }
    else if( interval_fossil_counts != NULL)
    {
        size_t num = interval_fossil_counts->getValue().size();

        if (interval >= num)
        {
            throw(RbException("Fossil count index out of bounds"));
        }
        return ascending ? interval_fossil_counts->getValue()[num - 1 - interval] : interval_fossil_counts->getValue()[interval];
    }
    else if( species_interval_fossil_counts != NULL )
    {
        size_t num = species_interval_fossil_counts->getValue().size();

        if (species >= num)
        {
            throw(RbException("Fossil count index out of bounds"));
        }

        num = species_interval_fossil_counts->getValue()[species].size();

        if (interval >= num)
        {
            throw(RbException("Fossil count index out of bounds"));
        }
        return ascending ? (int)species_interval_fossil_counts->getValue()[species][num - 1 - interval] : (int)species_interval_fossil_counts->getValue()[species][interval];
    }

    throw(RbException("Fossil counts have been marginalized"));
}


long PiecewiseConstantFossilizedBirthDeathRangeProcess::getFossilCount( size_t interval ) const
{

    // remove the old parameter first
    if ( fossil_counts != NULL )
    {
        return fossil_counts->getValue();
    }
    else if( interval_fossil_counts != NULL)
    {
        size_t num = interval_fossil_counts->getValue().size();

        if (interval >= num)
        {
            throw(RbException("Fossil count index out of bounds"));
        }
        return ascending ? interval_fossil_counts->getValue()[num - 1 - interval] : interval_fossil_counts->getValue()[interval];
    }
    else if( species_interval_fossil_counts != NULL )
    {
        size_t num = species_interval_fossil_counts->getValue().front().size();

        if (interval >= num)
        {
            throw(RbException("Fossil count index out of bounds"));
        }

        long total = 0;

        for(size_t i = 0; i < species_interval_fossil_counts->getValue().size(); i++)
        {
            total += ascending ? species_interval_fossil_counts->getValue()[i][num - 1 - interval] : species_interval_fossil_counts->getValue()[i][interval];
        }

        return total;
    }

    throw(RbException("Fossil counts have been marginalized"));
}


double PiecewiseConstantFossilizedBirthDeathRangeProcess::getFossilizationRate( size_t index ) const
{

    // remove the old parameter first
    if ( homogeneous_psi != NULL )
    {
        return homogeneous_psi->getValue();
    }
    else
    {
        size_t num = heterogeneous_psi->getValue().size();

        if (index >= num)
        {
            throw(RbException("Fossil sampling rate index out of bounds"));
        }
        return ascending ? heterogeneous_psi->getValue()[num - 1 - index] : heterogeneous_psi->getValue()[index];
    }
}


double PiecewiseConstantFossilizedBirthDeathRangeProcess::getIntervalTime( size_t index ) const
{

    if ( index == num_intervals - 1 )
    {
        return 0.0;
    }
    // remove the old parameter first
    else if ( timeline != NULL )
    {
        size_t num = timeline->getValue().size();

        if (index >= num)
        {
            throw(RbException("Interval time index out of bounds"));
        }
        return ascending ? timeline->getValue()[num - 1 - index] : timeline->getValue()[index];
    }
    else
    {
        throw(RbException("Interval time index out of bounds"));
    }
}


double PiecewiseConstantFossilizedBirthDeathRangeProcess::getSpeciationRate( size_t index ) const
{

    // remove the old parameter first
    if ( homogeneous_lambda != NULL )
    {
        return homogeneous_lambda->getValue();
    }
    else
    {
        size_t num = heterogeneous_lambda->getValue().size();

        if (index >= num)
        {
            throw(RbException("Speciation rate index out of bounds"));
        }
        return ascending ? heterogeneous_lambda->getValue()[num - 1 - index] : heterogeneous_lambda->getValue()[index];
    }
}


/**
 * \ln\int exp(psi t) q_tilde(t)/q(t) dt
 */
double PiecewiseConstantFossilizedBirthDeathRangeProcess::integrateQ( size_t i, double t ) const
{
    // get the parameters
    double b = birth[i];
    double d = death[i];
    double f = fossil[i];
    double r = (i == num_intervals - 1 ? homogeneous_rho->getValue() : 0.0);
    double ti = times[i];

    double diff = b - d - f;
    double bp   = b*f;
    double dt   = t - ti;

    double A = sqrt( diff*diff + 4.0*bp);
    double B = ( (1.0 - 2.0*(1.0-r)*p_i[i+1] )*b + d + f ) / A;

    double e = exp(-A*dt);

    double diff2 = b + d - f;
    double tmp = (1+B)/(A-diff2) - e*(1-B)/(A+diff2);
    double intQ = exp(-(diff2-A)*dt/2) * tmp;

//    double tmp = (1.0+B) + e*(1.0-B);
//    double q = 4.0*e / (tmp*tmp);
//    double q_tilde = sqrt(q*exp(-(b+d+f)*dt));
//    double p = b + d + f - A * ((1.0+B)-e*(1.0-B))/((1.0+B)+e*(1.0-B));
//    p /= 2*b;
//    double intQ = q_tilde/q * exp(f*dt) * ( b*p - f)/(b*d-f*d-b*f);

    return intQ;
}


/**
 * return the index i so that t_{i-1} > t >= t_i
 * where t_i is the instantaneous sampling time (i = 0,...,l)
 * t_0 is origin
 * t_l = 0.0
 */
size_t PiecewiseConstantFossilizedBirthDeathRangeProcess::l(double t) const
{
    return times.rend() - std::upper_bound( times.rbegin(), times.rend(), t);
}


/**
 * p_i(t)
 */
double PiecewiseConstantFossilizedBirthDeathRangeProcess::p( size_t i, double t ) const
{
    if ( t == 0) return 1.0;

    // get the parameters
    double b = birth[i];
    double d = death[i];
    double f = fossil[i];
    double r = (i == num_intervals - 1 ? homogeneous_rho->getValue() : 0.0);
    double ti = times[i];
    
    double diff = b - d - f;
    double bp   = b*f;
    double dt   = t - ti;
    
    double A = sqrt( diff*diff + 4.0*bp);
    double B = ( (1.0 - 2.0*(1.0-r)*p_i[i+1] )*b + d + f ) / A;
    
    double e = exp(-A*dt);
    double tmp = b + d + f - A * ((1.0+B)-e*(1.0-B))/((1.0+B)+e*(1.0-B));
    
    return tmp / (2.0*b);
}


/**
 * Compute the probability of survival if the process starts with one species at time start and ends at time end.
 *
 * \param[in]    start      Start time of the process.
 * \param[in]    end        End/stopping time of the process.
 *
 * \return Probability of survival.
 */
double PiecewiseConstantFossilizedBirthDeathRangeProcess::pSurvival(double start, double end) const
{
    double t = start;

    //std::vector<double> fossil_bak = fossil;

    //std::fill(fossil.begin(), fossil.end(), 0.0);

    double p0 = p(l(t), t);

    //fossil = fossil_bak;

    return 1.0 - p0;
}


/**
 * q_i(t)
 */
double PiecewiseConstantFossilizedBirthDeathRangeProcess::q( size_t i, double t, bool tilde ) const
{
    
    if ( t == 0.0 ) return 1.0;
    
    // get the parameters
    double b = birth[i];
    double d = death[i];
    double f = fossil[i];
    double r = (i == num_intervals - 1 ? homogeneous_rho->getValue() : 0.0);
    double ti = times[i];
    
    double diff = b - d - f;
    double bp   = b*f;
    double dt   = t - ti;

    double A = sqrt( diff*diff + 4.0*bp);
    double B = ( (1.0 - 2.0*(1.0-r)*p_i[i+1] )*b + d + f ) / A;

    double e = exp(-A*dt);
    double tmp = (1.0+B) + e*(1.0-B);

    double q = 4.0*e / (tmp*tmp);

    if (tilde) q = sqrt(q*exp(-(b+d+f)*dt));
    
    return q;
}


/**
 * Simulate new speciation times.
 */
void PiecewiseConstantFossilizedBirthDeathRangeProcess::redrawValue(void)
{
    // incorrect placeholder
    
    // Get the rng
    RandomNumberGenerator* rng = GLOBAL_RNG;
    
    double max = 0;
    // get the max first occurence
    for (size_t i = 0; i < taxa.size(); i++)
    {
        double o = taxa[i].getAgeRange().getMax();
        if ( o > max ) max = o;
    }
    
    max *= 1.1;
    
    if (max == 0.0)
    {
        max = 1.0;
    }

    // get random uniform draws
    for (size_t i = 0; i < taxa.size(); i++)
    {
        double b = taxa[i].getAgeRange().getMax() + rng->uniform01()*(max - taxa[i].getAgeRange().getMax());
        double d = rng->uniform01()*taxa[i].getAgeRange().getMin();

        (*this->value)[i][0] = b;
        (*this->value)[i][1] = d;
    }
}


void PiecewiseConstantFossilizedBirthDeathRangeProcess::keepSpecialization(DagNode *toucher)
{
    dirty_gamma = std::vector<bool>(taxa.size(), false);
}


void PiecewiseConstantFossilizedBirthDeathRangeProcess::restoreSpecialization(DagNode *toucher)
{

}


void PiecewiseConstantFossilizedBirthDeathRangeProcess::touchSpecialization(DagNode *toucher, bool touchAll)
{
    if ( toucher == dag_node )
    {
        std::set<size_t> touched_indices = dag_node->getTouchedElementIndices();

        for ( std::set<size_t>::iterator it = touched_indices.begin(); it != touched_indices.end(); it++)
        {
            size_t touched_range = (*it) / taxa.size();

            dirty_gamma[touched_range] = true;
        }
    }
}


/**
 *
 *
 */
void PiecewiseConstantFossilizedBirthDeathRangeProcess::updateIntervals( )
{
    std::vector<bool> youngest(taxa.size(), true);

    for (int i = (int)num_intervals - 1; i >= 0; i--)
    {
        double b = getSpeciationRate(i);
        double d = getExtinctionRate(i);
        double f = getFossilizationRate(i);
        double ti = getIntervalTime(i);

        birth[i] = b;
        death[i] = d;
        fossil[i] = f;
        times[i] = ti;

        if (i > 0)
        {

            double r = (i == num_intervals - 1 ? homogeneous_rho->getValue() : 0.0);
            double t = getIntervalTime(i-1);

            double diff = b - d - f;
            double dt   = t - ti;

            double A = sqrt( diff*diff + 4.0*b*f);
            double B = ( (1.0 - 2.0*(1.0-r)*p_i[i+1] )*b + d + f ) / A;

            double e = exp(-A*dt);

            double tmp = (1.0 + B) + e*(1.0 - B);

            q_i[i]       = 4.0*e / (tmp*tmp);
            q_tilde_i[i] = sqrt(q_i[i]*exp(-(b+d+f)*dt));
            p_i[i]       = (b + d + f - A * ((1.0+B)-e*(1.0-B))/tmp)/(2.0*b);
        }

        if( presence_absence )
        {
            for(size_t j = 0; j < taxa.size(); j++)
            {
                if( getFossilCount(i,j) > 0 )
                {
                    oldest_intervals[j] = i;
                    if( youngest[j] )
                    {
                        youngest_intervals[j] = i;
                        youngest[j] = false;
                    }
                }
            }
        }
    }
}


/**
 * Swap the parameters held by this distribution.
 * 
 * \param[in]    oldP      Pointer to the old parameter.
 * \param[in]    newP      Pointer to the new parameter.
 */
void PiecewiseConstantFossilizedBirthDeathRangeProcess::swapParameterInternal(const DagNode *oldP, const DagNode *newP)
{
    if (oldP == heterogeneous_lambda)
    {
        heterogeneous_lambda = static_cast<const TypedDagNode< RbVector<double> >* >( newP );
    }
    else if (oldP == heterogeneous_mu)
    {
        heterogeneous_mu = static_cast<const TypedDagNode< RbVector<double> >* >( newP );
    }
    else if (oldP == heterogeneous_psi)
    {
        heterogeneous_psi = static_cast<const TypedDagNode< RbVector<double> >* >( newP );
    }
    else if (oldP == homogeneous_lambda)
    {
        homogeneous_lambda = static_cast<const TypedDagNode<double>* >( newP );
    }
    else if (oldP == homogeneous_mu)
    {
        homogeneous_mu = static_cast<const TypedDagNode<double>* >( newP );
    }
    else if (oldP == homogeneous_psi)
    {
        homogeneous_psi = static_cast<const TypedDagNode<double>* >( newP );
    }
    else if (oldP == homogeneous_rho)
    {
        homogeneous_rho = static_cast<const TypedDagNode<double>* >( newP );
    }
    else if (oldP == timeline)
    {
        timeline = static_cast<const TypedDagNode< RbVector<double> >* >( newP );
    }
    else if (oldP == fossil_counts)
    {
        fossil_counts = static_cast<const TypedDagNode< long >* >( newP );
    }
    else if (oldP == interval_fossil_counts)
    {
        interval_fossil_counts = static_cast<const TypedDagNode< RbVector<long> >* >( newP );
    }
    else if (oldP == species_interval_fossil_counts)
    {
        species_interval_fossil_counts = static_cast<const TypedDagNode< RbVector<RbVector<long> > >* >( newP );
    }
}
