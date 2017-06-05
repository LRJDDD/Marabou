#include <cxxtest/TestSuite.h>

#include "MockErrno.h"
#include "Tableau.h"
#include <string.h>

class MockForTableau
{
public:
};

class TableauTestSuite : public CxxTest::TestSuite
{
public:
    MockForTableau *mock;

    void tableauUp()
    {
        TS_ASSERT( mock = new MockForTableau );
    }

    void tearDown()
    {
        TS_ASSERT_THROWS_NOTHING( delete mock );
    }

    void initializeTableauValues( Tableau &tableau )
    {
        /*
               | 3 2 1 2 1 0 0 | | x1 |   | 225 |
          Ax = | 1 1 1 1 0 1 0 | | x2 | = | 117 | = b
               | 4 3 3 4 0 0 1 | | x3 |   | 420 |
                                 | x4 |
                                 | x5 |
                                 | x6 |
                                 | x7 |

           x5 = 225 - 3x1 - 2x2 - x3  - 2x4
           x6 = 117 -  x1 -  x2 - x3  -  x4
           x7 = 420 - 4x1 - 3x2 - 3x3 - 4x4

        */

        tableau.setEntryValue( 0, 0, 3 );
        tableau.setEntryValue( 0, 1, 2 );
        tableau.setEntryValue( 0, 2, 1 );
        tableau.setEntryValue( 0, 3, 2 );
        tableau.setEntryValue( 0, 4, 1 );
        tableau.setEntryValue( 0, 5, 0 );
        tableau.setEntryValue( 0, 6, 0 );

        tableau.setEntryValue( 1, 0, 1 );
        tableau.setEntryValue( 1, 1, 1 );
        tableau.setEntryValue( 1, 2, 1 );
        tableau.setEntryValue( 1, 3, 1 );
        tableau.setEntryValue( 1, 4, 0 );
        tableau.setEntryValue( 1, 5, 1 );
        tableau.setEntryValue( 1, 6, 0 );

        tableau.setEntryValue( 2, 0, 4 );
        tableau.setEntryValue( 2, 1, 3 );
        tableau.setEntryValue( 2, 2, 3 );
        tableau.setEntryValue( 2, 3, 4 );
        tableau.setEntryValue( 2, 4, 0 );
        tableau.setEntryValue( 2, 5, 0 );
        tableau.setEntryValue( 2, 6, 1 );

        double b[3] = { 225, 117, 420 };
        tableau.setRightHandSide( b );
    }

    void test_initalize_basis_get_value()
    {
        Tableau *tableau;

        TS_ASSERT( tableau = new Tableau );

        TS_ASSERT_THROWS_NOTHING( tableau->setDimensions( 3, 7 ) );
        initializeTableauValues( *tableau );

        for ( unsigned i = 0; i < 4; ++i )
        {
            TS_ASSERT_THROWS_NOTHING( tableau->setLowerBound( i, 1 ) );
            TS_ASSERT_THROWS_NOTHING( tableau->setUpperBound( i, 2 ) );
        }

        TS_ASSERT_THROWS_NOTHING( tableau->setLowerBound( 4, 218 ) );
        TS_ASSERT_THROWS_NOTHING( tableau->setUpperBound( 4, 228 ) );

        TS_ASSERT_THROWS_NOTHING( tableau->setLowerBound( 5, 112 ) );
        TS_ASSERT_THROWS_NOTHING( tableau->setUpperBound( 5, 114 ) );

        TS_ASSERT_THROWS_NOTHING( tableau->setLowerBound( 6, 400 ) );
        TS_ASSERT_THROWS_NOTHING( tableau->setUpperBound( 6, 402 ) );

        Set<unsigned> basicVariables = { 4, 5, 6 };
        TS_ASSERT_THROWS_NOTHING( tableau->initializeBasis( basicVariables ) );

        // All non-basics are set to lower bounds.
        TS_ASSERT_EQUALS( tableau->getValue( 0 ), 1.0 );
        TS_ASSERT_EQUALS( tableau->getValue( 1 ), 1.0 );
        TS_ASSERT_EQUALS( tableau->getValue( 2 ), 1.0 );
        TS_ASSERT_EQUALS( tableau->getValue( 3 ), 1.0 );

        // Expect:
        // x5 = 225 - 3x1 - 2x2 - x3  - 2x4 = 225 - 8  = 217
        // x6 = 117 -  x1 -  x2 - x3  -  x4 = 117 - 4  = 113
        // x7 = 420 - 4x1 - 3x2 - 3x3 - 4x4 = 420 - 14 = 406

        TS_ASSERT_EQUALS( tableau->getValue( 4 ), 217.0 );
        TS_ASSERT_EQUALS( tableau->getValue( 5 ), 113.0 );
        TS_ASSERT_EQUALS( tableau->getValue( 6 ), 406.0 );

        TS_ASSERT_THROWS_NOTHING( delete tableau );
    }

    void test_initalize_basis_get_cost_function()
    {
        Tableau *tableau;

        TS_ASSERT( tableau = new Tableau );

        TS_ASSERT_THROWS_NOTHING( tableau->setDimensions( 3, 7 ) );
        initializeTableauValues( *tableau );

        for ( unsigned i = 0; i < 4; ++i )
        {
            TS_ASSERT_THROWS_NOTHING( tableau->setLowerBound( i, 1 ) );
            TS_ASSERT_THROWS_NOTHING( tableau->setUpperBound( i, 2 ) );
        }

        TS_ASSERT_THROWS_NOTHING( tableau->setLowerBound( 4, 218 ) );
        TS_ASSERT_THROWS_NOTHING( tableau->setUpperBound( 4, 228 ) );

        TS_ASSERT_THROWS_NOTHING( tableau->setLowerBound( 5, 112 ) );
        TS_ASSERT_THROWS_NOTHING( tableau->setUpperBound( 5, 114 ) );

        TS_ASSERT_THROWS_NOTHING( tableau->setLowerBound( 6, 400 ) );
        TS_ASSERT_THROWS_NOTHING( tableau->setUpperBound( 6, 402 ) );

        Set<unsigned> basicVariables = { 4, 5, 6 };
        TS_ASSERT_THROWS_NOTHING( tableau->initializeBasis( basicVariables ) );

        TS_ASSERT_EQUALS( tableau->getBasicStatus( 4 ), Tableau::BELOW_LB );
        TS_ASSERT_EQUALS( tableau->getBasicStatus( 5 ), Tableau::BETWEEN );
        TS_ASSERT_EQUALS( tableau->getBasicStatus( 6 ), Tableau::ABOVE_UB );

        const double *costFunction;
        TS_ASSERT_THROWS_NOTHING( costFunction = tableau->getCostFunction() );

        // Expect:
        // cost = - x5 + x7
        //      = + 3x1 + 2x2 +  x3 + 2x4
        //        - 4x1 - 3x2 - 3x3 - 4x4
        //      = -  x1 -  x2 - 2x3 - 2x4

        TS_ASSERT_EQUALS( costFunction[0], -1 );
        TS_ASSERT_EQUALS( costFunction[1], -1 );
        TS_ASSERT_EQUALS( costFunction[2], -2 );
        TS_ASSERT_EQUALS( costFunction[3], -2 );

        TS_ASSERT_THROWS_NOTHING( delete tableau );
    }

    void test_get_entering_variable()
    {
        Tableau *tableau;

        TS_ASSERT( tableau = new Tableau );

        TS_ASSERT_THROWS_NOTHING( tableau->setDimensions( 3, 7 ) );
        initializeTableauValues( *tableau );

        for ( unsigned i = 0; i < 4; ++i )
        {
            TS_ASSERT_THROWS_NOTHING( tableau->setLowerBound( i, 1 ) );
            TS_ASSERT_THROWS_NOTHING( tableau->setUpperBound( i, 2 ) );
        }

        TS_ASSERT_THROWS_NOTHING( tableau->setLowerBound( 4, 218 ) );
        TS_ASSERT_THROWS_NOTHING( tableau->setUpperBound( 4, 228 ) );

        TS_ASSERT_THROWS_NOTHING( tableau->setLowerBound( 5, 112 ) );
        TS_ASSERT_THROWS_NOTHING( tableau->setUpperBound( 5, 114 ) );

        TS_ASSERT_THROWS_NOTHING( tableau->setLowerBound( 6, 400 ) );
        TS_ASSERT_THROWS_NOTHING( tableau->setUpperBound( 6, 402 ) );

        Set<unsigned> basicVariables = { 4, 5, 6 };
        TS_ASSERT_THROWS_NOTHING( tableau->initializeBasis( basicVariables ) );

        TS_ASSERT_EQUALS( tableau->getBasicStatus( 4 ), Tableau::BELOW_LB );
        TS_ASSERT_EQUALS( tableau->getBasicStatus( 5 ), Tableau::BETWEEN );
        TS_ASSERT_EQUALS( tableau->getBasicStatus( 6 ), Tableau::ABOVE_UB );

        // Const function is: - x1 -  x2 - 2x3 - 2x4

        TS_ASSERT_THROWS_NOTHING( tableau->pickEnteringVariable() );
        TS_ASSERT_EQUALS( tableau->getEnteringVariable(), 2u );

        TS_ASSERT_THROWS_NOTHING( delete tableau );
    }

    void test_backward_transformation_no_eta()
    {
        Tableau tableau;

        TS_ASSERT_THROWS_NOTHING( tableau.setDimensions( 3, 7 ) );
        initializeTableauValues( tableau );

        Set<unsigned> basicVariables = { 4, 5, 6 };
        TS_ASSERT_THROWS_NOTHING( tableau.initializeBasis( basicVariables ) );

        double result[3];
        std::fill( result, result + 3, 0.0 );

        tableau.dump();

        TS_ASSERT_THROWS_NOTHING( tableau.backwardTransformation( 1, result ) );

        // In this case, B = I, and so we expect the result d to be the column a that
        // belongs to the entering variable, 1.
        TS_ASSERT_EQUALS( result[2], 3.0 );
        TS_ASSERT_EQUALS( result[1], 1.0 );
        TS_ASSERT_EQUALS( result[0], 2.0 );
    }

    void test_get_get_leaving_variable()
    {
        Tableau *tableau;

        TS_ASSERT( tableau = new Tableau );

        TS_ASSERT_THROWS_NOTHING( tableau->setDimensions( 3, 7 ) );
        initializeTableauValues( *tableau );

        for ( unsigned i = 0; i < 4; ++i )
        {
            TS_ASSERT_THROWS_NOTHING( tableau->setLowerBound( i, 1 ) );
            TS_ASSERT_THROWS_NOTHING( tableau->setUpperBound( i, 10 ) );
        }

        TS_ASSERT_THROWS_NOTHING( tableau->setLowerBound( 4, 219 ) );
        TS_ASSERT_THROWS_NOTHING( tableau->setUpperBound( 4, 228 ) );

        TS_ASSERT_THROWS_NOTHING( tableau->setLowerBound( 5, 112 ) );
        TS_ASSERT_THROWS_NOTHING( tableau->setUpperBound( 5, 114 ) );

        TS_ASSERT_THROWS_NOTHING( tableau->setLowerBound( 6, 400 ) );
        TS_ASSERT_THROWS_NOTHING( tableau->setUpperBound( 6, 402 ) );

        Set<unsigned> basicVariables = { 4, 5, 6 };
        TS_ASSERT_THROWS_NOTHING( tableau->initializeBasis( basicVariables ) );

        TS_ASSERT_EQUALS( tableau->getBasicStatus( 4 ), Tableau::BELOW_LB );
        TS_ASSERT_EQUALS( tableau->getBasicStatus( 5 ), Tableau::BETWEEN );
        TS_ASSERT_EQUALS( tableau->getBasicStatus( 6 ), Tableau::ABOVE_UB );

        TS_ASSERT_THROWS_NOTHING( tableau->pickEnteringVariable() );
        TS_ASSERT_EQUALS( tableau->getEnteringVariable(), 2u );

        // Entering variable is 2, and it needs to increase
        // Current basic values are: 217, 113, 406
        TS_ASSERT_EQUALS( tableau->getValue( 4 ), 217.0 );
        TS_ASSERT_EQUALS( tableau->getValue( 5 ), 113.0 );
        TS_ASSERT_EQUALS( tableau->getValue( 6 ), 406.0 );

        double d1[] = { 1, 1, 1 };
        // Var 4 will hit its lower bound: constraint is 2
        // Var 5 will hit its upper bound: constraint is 1
        // Var 6 poses no constraint
        TS_ASSERT_THROWS_NOTHING( tableau->pickLeavingVariable( d1 ) );
        TS_ASSERT_EQUALS( tableau->getLeavingVariable(), 5u );
        TS_ASSERT_EQUALS( tableau->getChangeRatio(), 1.0 );

        double d2[] = { 0.5, 0.5, 0.5 };
        // d1 scaled by 1/2
        TS_ASSERT_THROWS_NOTHING( tableau->pickLeavingVariable( d2 ) );
        TS_ASSERT_EQUALS( tableau->getLeavingVariable(), 5u );
        TS_ASSERT_EQUALS( tableau->getChangeRatio(), 2.0 );

        double d3[] = { -1, -1, -1 };
        // Var 4 poses no constraint
        // Var 5 will hit its lower bound: constraint is 1
        // Var 6 will hit its upper bound: constraint is 4
        TS_ASSERT_THROWS_NOTHING( tableau->pickLeavingVariable( d3 ) );
        TS_ASSERT_EQUALS( tableau->getLeavingVariable(), 5u );
        TS_ASSERT_EQUALS( tableau->getChangeRatio(), 1.0 );

        double d4[] = { -1, -0.1, -2 };
        // Var 4 poses no constraint
        // Var 5 will hit its lower bound: constraint is 10
        // Var 6 will hit its upper bound: constraint is 2
        TS_ASSERT_THROWS_NOTHING( tableau->pickLeavingVariable( d4 ) );
        TS_ASSERT_EQUALS( tableau->getLeavingVariable(), 6u );
        TS_ASSERT_EQUALS( tableau->getChangeRatio(), 2.0 );

        double d5[] = { -1, 0, -0.5 };
        // Var 4 poses no constraint
        // Var 5 poses no constraint
        // Var 6 will hit its upper bound: constraint is 8
        TS_ASSERT_THROWS_NOTHING( tableau->pickLeavingVariable( d5 ) );
        TS_ASSERT_EQUALS( tableau->getLeavingVariable(), 6u );
        TS_ASSERT_EQUALS( tableau->getChangeRatio(), 8.0 );

        double d6[] = { 0.5, 0, -0.1 };
        // Var 4 will hit its lower bound: constraint is 4
        // Var 5 poses no constraint
        // Var 6 will hit its upper bound: constraint is 40
        TS_ASSERT_THROWS_NOTHING( tableau->pickLeavingVariable( d6 ) );
        TS_ASSERT_EQUALS( tableau->getLeavingVariable(), 4u );
        TS_ASSERT_EQUALS( tableau->getChangeRatio(), 4.0 );

        double d7[] = { -1, 0, -0.00001 };
        // The entering variable (2) can change by 9 at most. Here
        // this will be the bound.
        TS_ASSERT_THROWS_NOTHING( tableau->pickLeavingVariable( d7 ) );
        TS_ASSERT_EQUALS( tableau->getLeavingVariable(), 2u );
        TS_ASSERT_EQUALS( tableau->getChangeRatio(), 9.0 );

        TS_ASSERT_THROWS_NOTHING( delete tableau );
    }

    void test_perform_pivot_nonbasic_goes_to_opposite_bound()
    {
        Tableau *tableau;

        TS_ASSERT( tableau = new Tableau );

        TS_ASSERT_THROWS_NOTHING( tableau->setDimensions( 3, 7 ) );
        initializeTableauValues( *tableau );

        for ( unsigned i = 0; i < 4; ++i )
        {
            TS_ASSERT_THROWS_NOTHING( tableau->setLowerBound( i, 1 ) );
            TS_ASSERT_THROWS_NOTHING( tableau->setUpperBound( i, 10 ) );
        }

        TS_ASSERT_THROWS_NOTHING( tableau->setLowerBound( 4, 219 ) );
        TS_ASSERT_THROWS_NOTHING( tableau->setUpperBound( 4, 228 ) );

        TS_ASSERT_THROWS_NOTHING( tableau->setLowerBound( 5, 112 ) );
        TS_ASSERT_THROWS_NOTHING( tableau->setUpperBound( 5, 114 ) );

        TS_ASSERT_THROWS_NOTHING( tableau->setLowerBound( 6, 400 ) );
        TS_ASSERT_THROWS_NOTHING( tableau->setUpperBound( 6, 402 ) );

        Set<unsigned> basicVariables = { 4, 5, 6 };
        TS_ASSERT_THROWS_NOTHING( tableau->initializeBasis( basicVariables ) );

        TS_ASSERT_THROWS_NOTHING( tableau->pickEnteringVariable() );

        double d[] = { -1, 0, -0.00001 };
        TS_ASSERT_THROWS_NOTHING( tableau->pickLeavingVariable( d ) );
        TS_ASSERT_EQUALS( tableau->getLeavingVariable(), 2u );
        TS_ASSERT_EQUALS( tableau->getEnteringVariable(), 2u );

        TS_ASSERT( !tableau->isBasic( 2u ) );

        // Before the pivot, the variable is at the lower bound
        TS_ASSERT_EQUALS( tableau->getValue( 2u ), 1.0 );
        TS_ASSERT_THROWS_NOTHING( tableau->performPivot() );
        // After the pivot, the variable is at the upper bound
        TS_ASSERT_EQUALS( tableau->getValue( 2u ), 10.0 );

        TS_ASSERT( !tableau->isBasic( 2u ) );

        TS_ASSERT_THROWS_NOTHING( delete tableau );
    }

    void test_perform_pivot_nonbasic_becomes_basic()
    {
        Tableau *tableau;

        TS_ASSERT( tableau = new Tableau );

        TS_ASSERT_THROWS_NOTHING( tableau->setDimensions( 3, 7 ) );
        initializeTableauValues( *tableau );

        for ( unsigned i = 0; i < 4; ++i )
        {
            TS_ASSERT_THROWS_NOTHING( tableau->setLowerBound( i, 1 ) );
            TS_ASSERT_THROWS_NOTHING( tableau->setUpperBound( i, 10 ) );
        }

        TS_ASSERT_THROWS_NOTHING( tableau->setLowerBound( 4, 219 ) );
        TS_ASSERT_THROWS_NOTHING( tableau->setUpperBound( 4, 228 ) );

        TS_ASSERT_THROWS_NOTHING( tableau->setLowerBound( 5, 112 ) );
        TS_ASSERT_THROWS_NOTHING( tableau->setUpperBound( 5, 114 ) );

        TS_ASSERT_THROWS_NOTHING( tableau->setLowerBound( 6, 400 ) );
        TS_ASSERT_THROWS_NOTHING( tableau->setUpperBound( 6, 402 ) );

        Set<unsigned> basicVariables = { 4, 5, 6 };
        TS_ASSERT_THROWS_NOTHING( tableau->initializeBasis( basicVariables ) );

        TS_ASSERT_THROWS_NOTHING( tableau->pickEnteringVariable() );

        double d[] = { 1, 1, 1 };
        // Var 4 will hit its lower bound: constraint is 2
        // Var 5 will hit its upper bound: constraint is 1
        // Var 6 poses no constraint
        TS_ASSERT_THROWS_NOTHING( tableau->pickLeavingVariable( d ) );
        TS_ASSERT_EQUALS( tableau->getEnteringVariable(), 2u );
        TS_ASSERT_EQUALS( tableau->getLeavingVariable(), 5u );

        TS_ASSERT( !tableau->isBasic( 2u ) );
        TS_ASSERT( tableau->isBasic( 5u ) );

        TS_ASSERT_EQUALS( tableau->getValue( 2u ), 1.0 );
        TS_ASSERT_EQUALS( tableau->getValue( 5u ), 113.0 );

        TS_ASSERT_THROWS_NOTHING( tableau->performPivot() );

        TS_ASSERT( tableau->isBasic( 2u ) );
        TS_ASSERT( !tableau->isBasic( 5u ) );

        // The new non-basic variable is at its upper bound
        TS_ASSERT_EQUALS( tableau->getValue( 5u ), 114.0 );

        TS_ASSERT_THROWS_NOTHING( delete tableau );
    }
};

//
// Local Variables:
// compile-command: "make -C ../.. "
// tags-file-name: "../../TAGS"
// c-basic-offset: 4
// End:
//
