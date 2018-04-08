#include "ModelVector.h"
#include "Natural.h"
#include "RbUtil.h"
#include "RlBoolean.h"
#include "RlClade.h"
#include "RlTree.h"
#include "RlMemberFunction.h"
#include "RlString.h"
#include "RlTaxon.h"
#include "RealPos.h"
#include "TopologyNode.h"
#include "TreeUtilities.h"
#include "TypeSpec.h"

#include <sstream>

using namespace RevLanguage;

/** Default constructor */
Tree::Tree(void) : ModelObject<RevBayesCore::Tree>()
{

    initMethods();

}

/** Construct from core pointer */
Tree::Tree(RevBayesCore::Tree *t) : ModelObject<RevBayesCore::Tree>( t )
{

    initMethods();

}

/** Construct from core reference */
Tree::Tree(const RevBayesCore::Tree &t) : ModelObject<RevBayesCore::Tree>( new RevBayesCore::Tree( t ) )
{

    initMethods();

}

/** Construct from core DAG node */
Tree::Tree(RevBayesCore::TypedDagNode<RevBayesCore::Tree> *n) : ModelObject<RevBayesCore::Tree>( n )
{

    initMethods();

}


/**
 * The clone function is a convenience function to create proper copies of inherited objected.
 * E.g. a.clone() will create a clone of the correct type even if 'a' is of derived type 'b'.
 *
 * \return A new copy of the process.
 */
Tree* Tree::clone(void) const
{

    return new Tree(*this);
}


/* Map calls to member methods */
RevLanguage::RevPtr<RevLanguage::RevVariable> Tree::executeMethod(std::string const &name, const std::vector<Argument> &args, bool &found)
{

    if (name == "dropTip")
    {
        found = true;

        const RevObject &taxon = args[0].getVariable()->getRevObject();
        std::string taxon_name = "";
        if ( taxon.isType( RlString::getClassTypeSpec() ) )
        {
            taxon_name = static_cast<const RlString&>( taxon ).getValue();
        }
        else
        {
            taxon_name = static_cast<const Taxon&>( taxon ).getValue().getSpeciesName();
        }

        this->dag_node->getValue().dropTipNodeWithName( taxon_name );

        return NULL;
    }
    else if (name == "isInternal")
    {
        found = true;

        long index = static_cast<const Natural&>( args[0].getVariable()->getRevObject() ).getValue() - 1;

        bool tf = this->dag_node->getValue().getNode((size_t)index).isInternal();
        return new RevVariable( new RlBoolean( tf ) );
    }
    else if (name == "names" || name == "taxa")
    {
        found = true;

        std::vector<RevBayesCore::Taxon> t = this->dag_node->getValue().getTaxa();
        return new RevVariable( new ModelVector<Taxon>( t ) );
    }
    else if (name == "setTaxonName")
    {
        found = true;
        
        const RevObject& current = args[0].getVariable()->getRevObject();
        if ( current.isType( RlString::getClassTypeSpec() ) )
        {
            std::string n = std::string( static_cast<const RlString&>( current ).getValue() );
            const RevObject& new_name = args[1].getVariable()->getRevObject();
            if ( new_name.isType( RlString::getClassTypeSpec() ) )
            {
                std::string name = std::string( static_cast<const RlString&>( new_name ).getValue() );
                getDagNode()->getValue().setTaxonName( n ,name );
                // std::cout << "new name: "<< dagNode->getValue().getTaxonData( n ).getTaxonName() << std::endl;
            }
        }
        return NULL;
    }
    else if (name == "nodeName")
    {
        found = true;

        long index = static_cast<const Natural&>( args[0].getVariable()->getRevObject() ).getValue() - 1;
        const std::string& n = this->dag_node->getValue().getNode((size_t)index).getName();
        return new RevVariable( new RlString( n ) );
    }
    else if (name == "removeDuplicateTaxa")
    {
        found = true;

        RevBayesCore::Tree &tree = dag_node->getValue();
        tree.removeDuplicateTaxa();
        
        return NULL;
    }
    else if (name == "rescale")
    {
        found = true;
        
        double f = static_cast<const RealPos&>( args[0].getVariable()->getRevObject() ).getValue();
        RevBayesCore::Tree &tree = dag_node->getValue();
        RevBayesCore::TreeUtilities::rescaleTree(&tree, &tree.getRoot(), f);
        
        return NULL;
    }
    else if (name == "offset")
    {
        found = true;

        double f = static_cast<const RealPos&>( args[0].getVariable()->getRevObject() ).getValue();
        RevBayesCore::Tree &tree = dag_node->getValue();
        RevBayesCore::TreeUtilities::offsetTree(&tree, &tree.getRoot(), f);

        return NULL;
    }
    else if (name == "setNegativeConstraint")
    {
        found = true;

        double tf = static_cast<const RlBoolean&>( args[0].getVariable()->getRevObject() ).getValue();
        RevBayesCore::Tree &tree = dag_node->getValue();
        tree.setNegativeConstraint(tf);
//        RevBayesCore::TreeUtilities::rescaleTree(&tree, &tree.getRoot(), f);

        return NULL;
    }
    else if (name == "tipIndex")
    {
        found = true;
        
        std::string tip_name = "";
        if ( args[0].getVariable()->getRevObject().getType() == RlString::getClassType() )
        {
            tip_name = static_cast<const RlString&>( args[0].getVariable()->getRevObject() ).getValue();
        }
        else if ( args[0].getVariable()->getRevObject().getType() == Taxon::getClassType() )
        {
            tip_name = static_cast<const Taxon&>( args[0].getVariable()->getRevObject() ).getValue().getSpeciesName();
        }
        long index = this->dag_node->getValue().getTipNodeWithName( tip_name ).getIndex() + 1;
        return new RevVariable( new Natural( index ) );
    }
    else if (name == "makeUltrametric")
    {

        found = true;

        RevBayesCore::Tree &tree = dag_node->getValue();
        RevBayesCore::TreeUtilities::makeUltrametric(&tree);

//        tree.makeUltrametric();

        return NULL;
    }

    return ModelObject<RevBayesCore::Tree>::executeMethod( name, args, found );
}


/** Get Rev type of object */
const std::string& Tree::getClassType(void)
{

    static std::string rev_type = "Tree";

    return rev_type;
}

/** Get class type spec describing type of object */
const TypeSpec& Tree::getClassTypeSpec(void)
{

    static TypeSpec rev_type_spec = TypeSpec( getClassType(), new TypeSpec( RevObject::getClassTypeSpec() ) );

    return rev_type_spec;
}


/** Get type spec */
const TypeSpec& Tree::getTypeSpec( void ) const
{

    static TypeSpec type_spec = getClassTypeSpec();

    return type_spec;
}


/**
 * Initialize the member methods.
 */
void Tree::initMethods( void )
{

    ArgumentRules* isInternalArgRules = new ArgumentRules();
    isInternalArgRules->push_back( new ArgumentRule( "node", Natural::getClassTypeSpec(), "The index of the node.", ArgumentRule::BY_VALUE, ArgumentRule::ANY ) );
    methods.addFunction( new MemberProcedure( "isInternal", RlBoolean::getClassTypeSpec(), isInternalArgRules ) );

    ArgumentRules* same_topology_arg_rules = new ArgumentRules();
    same_topology_arg_rules->push_back(        new ArgumentRule("tree"    , Tree::getClassTypeSpec(), "The reference tree.", ArgumentRule::BY_CONSTANT_REFERENCE, ArgumentRule::ANY ) );
    methods.addFunction( new MemberFunction<Tree, RlBoolean>( "hasSameTopology", this, same_topology_arg_rules ) );
    
    ArgumentRules* nnodesArgRules = new ArgumentRules();
    methods.addFunction( new MemberFunction<Tree, Natural>( "nnodes", this, nnodesArgRules ) );

    ArgumentRules* ntipsArgRules = new ArgumentRules();
    methods.addFunction( new MemberFunction<Tree, Natural>( "ntips", this, ntipsArgRules ) );

    ArgumentRules* namesArgRules = new ArgumentRules();
    methods.addFunction( new MemberProcedure( "names", ModelVector<RlString>::getClassTypeSpec(), namesArgRules ) );

    ArgumentRules* taxaArgRules = new ArgumentRules();
    methods.addFunction( new MemberProcedure( "taxa", ModelVector<Taxon>::getClassTypeSpec(), taxaArgRules ) );
    
    ArgumentRules* setTaxonNameArgRules         = new ArgumentRules();
    setTaxonNameArgRules->push_back(        new ArgumentRule("current"    , RlString::getClassTypeSpec(), "The old name.", ArgumentRule::BY_VALUE, ArgumentRule::ANY ) );
    setTaxonNameArgRules->push_back(        new ArgumentRule("new"        , RlString::getClassTypeSpec(), "The new name.", ArgumentRule::BY_VALUE, ArgumentRule::ANY ) );
    methods.addFunction( new MemberProcedure( "setTaxonName", RlUtils::Void, setTaxonNameArgRules ) );

    ArgumentRules* nodeNameArgRules = new ArgumentRules();
    nodeNameArgRules->push_back( new ArgumentRule( "node", Natural::getClassTypeSpec(), "The index of the node.", ArgumentRule::BY_CONSTANT_REFERENCE, ArgumentRule::ANY ) );
    methods.addFunction( new MemberProcedure( "nodeName", RlString::getClassTypeSpec(),  nodeNameArgRules ) );
    
    ArgumentRules* tip_index_arg_rules = new ArgumentRules();
    std::vector<TypeSpec> tip_index_arg_types;
    tip_index_arg_types.push_back( RlString::getClassTypeSpec() );
    tip_index_arg_types.push_back( Taxon::getClassTypeSpec() );
    tip_index_arg_rules->push_back( new ArgumentRule( "name", tip_index_arg_types, "The name of the tip/taxon.", ArgumentRule::BY_CONSTANT_REFERENCE, ArgumentRule::ANY ) );
    methods.addFunction( new MemberProcedure( "tipIndex", Natural::getClassTypeSpec(),  tip_index_arg_rules ) );

    ArgumentRules* drop_tip_arg_rules = new ArgumentRules();
    std::vector<TypeSpec> tip_types;
    tip_types.push_back( RlString::getClassTypeSpec() );
    tip_types.push_back( Taxon::getClassTypeSpec() );
    drop_tip_arg_rules->push_back( new ArgumentRule( "node", tip_types, "The index of the node.", ArgumentRule::BY_CONSTANT_REFERENCE, ArgumentRule::ANY ) );
    methods.addFunction( new MemberProcedure( "dropTip", RlUtils::Void,  drop_tip_arg_rules ) );


    ArgumentRules* rescaleArgRules = new ArgumentRules();
    rescaleArgRules->push_back( new ArgumentRule( "factor", RealPos::getClassTypeSpec(), "The scaling factor.", ArgumentRule::BY_VALUE, ArgumentRule::ANY ) );
    methods.addFunction( new MemberProcedure( "rescale", RlUtils::Void, rescaleArgRules ) );
    
    
    ArgumentRules* remove_duplicate_taxa_arg_rules = new ArgumentRules();
    methods.addFunction( new MemberProcedure( "removeDuplicateTaxa", RlUtils::Void, remove_duplicate_taxa_arg_rules ) );
    

    ArgumentRules* offsetArgRules = new ArgumentRules();
    offsetArgRules->push_back( new ArgumentRule( "factor", RealPos::getClassTypeSpec(), "The offset factor.", ArgumentRule::BY_VALUE, ArgumentRule::ANY ) );
    methods.addFunction( new MemberProcedure( "offset", RlUtils::Void, offsetArgRules ) );

    ArgumentRules* setNegativeConstraint = new ArgumentRules();
    setNegativeConstraint->push_back( new ArgumentRule( "flag", RlBoolean::getClassTypeSpec(), "Is the tree a negative constraint?.", ArgumentRule::BY_VALUE, ArgumentRule::ANY ) );
    methods.addFunction( new MemberProcedure( "setNegativeConstraint", RlUtils::Void, setNegativeConstraint ) );

    ArgumentRules* makeUltraArgRules = new ArgumentRules();
    methods.addFunction( new MemberProcedure( "makeUltrametric", RlUtils::Void, makeUltraArgRules ) );


    // member functions
    ArgumentRules* parentArgRules = new ArgumentRules();
    parentArgRules->push_back( new ArgumentRule( "node", Natural::getClassTypeSpec(), "The index of the node.", ArgumentRule::BY_CONSTANT_REFERENCE, ArgumentRule::ANY ) );
    methods.addFunction( new MemberFunction<Tree, Natural>( "parent", this, parentArgRules   ) );

    ArgumentRules* childArgRules = new ArgumentRules();
    childArgRules->push_back( new ArgumentRule( "node", Natural::getClassTypeSpec(), "The index of the node.", ArgumentRule::BY_CONSTANT_REFERENCE, ArgumentRule::ANY ) );
    childArgRules->push_back( new ArgumentRule( "index", Natural::getClassTypeSpec(), "The index of the child of this node.", ArgumentRule::BY_CONSTANT_REFERENCE, ArgumentRule::ANY ) );
    methods.addFunction( new MemberFunction<Tree, Natural>( "child", this, childArgRules   ) );

    ArgumentRules* branchLengthArgRules = new ArgumentRules();
    branchLengthArgRules->push_back( new ArgumentRule( "node", Natural::getClassTypeSpec(), "The index of the node.", ArgumentRule::BY_CONSTANT_REFERENCE, ArgumentRule::ANY ) );
    methods.addFunction( new MemberFunction<Tree, RealPos>( "branchLength", this, branchLengthArgRules   ) );

    ArgumentRules* contained_in_clade_arg_rules = new ArgumentRules();
    contained_in_clade_arg_rules->push_back( new ArgumentRule( "node" , Natural::getClassTypeSpec(), "The index of the node.", ArgumentRule::BY_CONSTANT_REFERENCE, ArgumentRule::ANY ) );
    contained_in_clade_arg_rules->push_back( new ArgumentRule( "clade", Clade::getClassTypeSpec()  , "The embracing clade.", ArgumentRule::BY_VALUE, ArgumentRule::ANY ) );
    methods.addFunction( new MemberFunction<Tree, RlBoolean>( "isContainedInClade", this, contained_in_clade_arg_rules ) );

    ArgumentRules* contains_clade_arg_rules = new ArgumentRules();
    contains_clade_arg_rules->push_back( new ArgumentRule( "clade", Clade::getClassTypeSpec()  , "The embracing clade.", ArgumentRule::BY_VALUE, ArgumentRule::ANY ) );
    methods.addFunction( new MemberFunction<Tree, RlBoolean>( "containsClade", this, contains_clade_arg_rules ) );

    ArgumentRules* treeLengthArgRules = new ArgumentRules();
    methods.addFunction( new MemberFunction<Tree, RealPos>( "treeLength", this, treeLengthArgRules   ) );


}
