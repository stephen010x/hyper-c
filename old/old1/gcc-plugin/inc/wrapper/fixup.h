#ifndef FIXUP_H


// FROM /usr/lib/gcc/x86_64-linux-gnu/15/plugin/include/hashtab.h:142
enum insert_option {NO_INSERT, INSERT};



// FROM /usr/lib/gcc/x86_64-linux-gnu/15/plugin/include/plugin.h:159
struct attribute_spec;
// FROM /usr/lib/gcc/x86_64-linux-gnu/15/plugin/include/tree-core.h:2203
/* Structure describing an attribute and a function to handle it.  */
struct attribute_spec {
  /* The name of the attribute (without any leading or trailing __),
     or NULL to mark the end of a table of attributes.  */
  const char *name;
  /* The minimum length of the list of arguments of the attribute.  */
  int min_length;
  /* The maximum length of the list of arguments of the attribute
     (-1 for no maximum).  It can also be -2 for fake attributes
     created for the sake of -Wno-attributes; in that case, we
     should skip the balanced token sequence when parsing the attribute.  */
  int max_length;
  /* Whether this attribute requires a DECL.  If it does, it will be passed
     from types of DECLs, function return types and array element types to
     the DECLs, function types and array types respectively; but when
     applied to a type in any other circumstances, it will be ignored with
     a warning.  (If greater control is desired for a given attribute,
     this should be false, and the flags argument to the handler may be
     used to gain greater control in that case.)  */
  bool decl_required;
  /* Whether this attribute requires a type.  If it does, it will be passed
     from a DECL to the type of that DECL.  */
  bool type_required;
  /* Whether this attribute requires a function (or method) type.  If it does,
     it will be passed from a function pointer type to the target type,
     and from a function return type (which is not itself a function
     pointer type) to the function type.  */
  bool function_type_required;
  /* Specifies if attribute affects type's identity.  */
  bool affects_type_identity;
  /* Function to handle this attribute.  NODE points to a tree[3] array,
     where node[0] is the node to which the attribute is to be applied;
     node[1] is the last pushed/merged declaration if one exists, and node[2]
     may be the declaration for node[0].  If a DECL, it should be modified in
     place; if a TYPE, a copy should be created.  NAME is the canonicalized
     name of the attribute i.e. without any leading or trailing underscores.
     ARGS is the TREE_LIST of the arguments (which may be NULL).  FLAGS gives
     further information about the context of the attribute.  Afterwards, the
     attributes will be added to the DECL_ATTRIBUTES or TYPE_ATTRIBUTES, as
     appropriate, unless *NO_ADD_ATTRS is set to true (which should be done on
     error, as well as in any other cases when the attributes should not be
     added to the DECL or TYPE).  Depending on FLAGS, any attributes to be
     applied to another type or DECL later may be returned;
     otherwise the return value should be NULL_TREE.  This pointer may be
     NULL if no special handling is required beyond the checks implied
     by the rest of this structure.  */
  /*tree (*handler) (tree *node, tree name, tree args,
		   int flags, bool *no_add_attrs);*/

  /* Specifies the name of an attribute that's mutually exclusive with
     this one, and whether the relationship applies to the function,
     variable, or type form of the attribute.  */
  struct exclusions {
    const char *name;
    bool function;
    bool variable;
    bool type;
  } const *exclude;

  /* An array of attribute exclusions describing names of other attributes
     that this attribute is mutually exclusive with.  */
  //const exclusions *exclude;
};







// FROM /usr/lib/gcc/x86_64-linux-gnu/15/plugin/include/plugin.h:159



struct array_slice_const_attribute_spec {
    const struct attribute_spec *m_base;
    unsigned int m_size;
};




// FROM /usr/lib/gcc/x86_64-linux-gnu/15/plugin/include/attribs.h:24
/* A set of attributes that belong to the same namespace, given by NS.  */
struct scoped_attribute_spec
{
  const char *ns;
  struct array_slice_const_attribute_spec attributes;
};





#endif /* #ifndef FIXUP_H */
