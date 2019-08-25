/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "ITS-Container"
 * 	found in "CDD_TS102894-2/v1.2.1/ITS-Container.asn"
 * 	`asn1c -fcompound-names -fincludes-quoted -no-gen-example`
 */

#include "StationID.h"

int
StationID_constraint(const asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	
	if(!sptr) {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	
	/* Constraint check succeeded */
	return 0;
}

/*
 * This type is implemented using NativeInteger,
 * so here we adjust the DEF accordingly.
 */
static asn_oer_constraints_t asn_OER_type_StationID_constr_1 CC_NOTUSED = {
	{ 4, 1 }	/* (0..4294967295) */,
	-1};
asn_per_constraints_t asn_PER_type_StationID_constr_1 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 32, -1,  0,  4294967295 }	/* (0..4294967295) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
const asn_INTEGER_specifics_t asn_SPC_StationID_specs_1 = {
	0,	0,	0,	0,	0,
	0,	/* Native long size */
	1	/* Unsigned representation */
};
static const ber_tlv_tag_t asn_DEF_StationID_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (2 << 2))
};
asn_TYPE_descriptor_t asn_DEF_StationID = {
	"StationID",
	"StationID",
	&asn_OP_NativeInteger,
	asn_DEF_StationID_tags_1,
	sizeof(asn_DEF_StationID_tags_1)
		/sizeof(asn_DEF_StationID_tags_1[0]), /* 1 */
	asn_DEF_StationID_tags_1,	/* Same as above */
	sizeof(asn_DEF_StationID_tags_1)
		/sizeof(asn_DEF_StationID_tags_1[0]), /* 1 */
	{ &asn_OER_type_StationID_constr_1, &asn_PER_type_StationID_constr_1, StationID_constraint },
	0, 0,	/* No members */
	&asn_SPC_StationID_specs_1	/* Additional specs */
};

