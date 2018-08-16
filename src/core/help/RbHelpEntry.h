#ifndef RbHelpEntry_H
#define RbHelpEntry_H

#include <string>
#include <vector>

#include "RbHelpReference.h"

namespace RevBayesCore {
    
    
    /**
     * \brief The help entry base class.
     *
     * A help entry for a type provides all the help information available on this type, e.g.,
     * about member methods.
     *
     * \copyright (c) Copyright 2009-2013 (GPL version 3)
     * \author The RevBayes Development Core Team (Johan Dunfalk & Sebastian Hoehna)
     * \since Version 1.0, 2014-09-15
     *
     */
    class RbHelpEntry {
        
    public:
        
        virtual                                     ~RbHelpEntry() {}
        
        // getter
        const std::vector<std::string>&             getAliases(void) const;
        const std::vector<std::string>&             getAuthor(void) const;
        const std::string&                          getCategoryType(void) const;
        const std::vector<std::string>&             getDescription(void) const;
        const std::vector<std::string>&             getDetails(void) const;
        const std::string&                          getExample(void) const;
        const std::string&                          getName(void) const;
        const std::vector<RbHelpReference>&         getReferences(void) const;
        const std::vector<std::string>&             getSeeAlso(void) const;
        const std::string&                          getTitle(void) const;
        
        // setters
        void                                        setAliases(const std::vector<std::string> &a);
        void                                        setAuthor(const std::vector<std::string> &a);
        void                                        setCategoryType(const std::string &a);
        void                                        setDetails(const std::vector<std::string> &d);
        void                                        setDescription(const std::vector<std::string> &d);
        void                                        setExample(const std::string &e);
        void                                        setName(const std::string &n);
        void                                        setReferences(const std::vector<RbHelpReference> &r);
        void                                        setSeeAlso(const std::vector<std::string> &s);
        void                                        setTitle(const std::string &t);
        
        
    private:
        
        std::vector<std::string>                    aliases;
        std::vector<std::string>                    author;
        std::string                                 categoryType;
        std::vector<std::string>                    description;
        std::vector<std::string>                    details;
        std::string                                 example;
        std::string                                 name;
        std::vector<RbHelpReference>                references;
        std::vector<std::string>                    seeAlso;
        std::string                                 title;
        
    };
    
    
}

#endif

