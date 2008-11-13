#include "AE_General.r"

resource 'PiPL' (16000) {
	{	/* array properties: 12 elements */
		/* [1] */
		Kind {
			AEEffect
		},
		/* [2] */
		Name {
			"Ampede PDF"
		},
		/* [3] */
		Category {
			"Ampede"
		},
		
		/* [4] */
		CodeCarbonPowerPC {
			0,
			0,
			""
		},
		CodePowerPC {
			0,
			0,
			""
		},

		/* [6] */
		AE_PiPL_Version {
			2,
			0
		},
		/* [7] */
		AE_Effect_Spec_Version {
			12,
			10
		},
		/* [8] */
		AE_Effect_Version {
			1048577	/* 2.0 */
		},
		/* [9] */
		AE_Effect_Info_Flags {
			0
		},
		/* [10] */
		AE_Effect_Global_OutFlags {
			/* 1088  0b10001000000 = ouptutExtent | pixIndep; */
			/* 1120  0b10101100000 = dialog | ouptutExtent | pixIndep */
			/* 1376  0b10101100000 = dialog | ouptutExtent | pixIndep | expandBuffer; */
			16777268 /*  0b1000000000000000000110100 = PF_OutFlag_NON_PARAM_VARY | PF_OutFlag_SEQUENCE_DATA_NEEDS_FLATTENING | PF_OutFlag_I_DO_DIALOG | PF_OutFlag_I_HAVE_EXTERNAL_DEPENDENCIES */
		},
		/* [11] */
		AE_Effect_Match_Name {
			"AmpedeFCP"
		},
		/* [12] */
		AE_Reserved_Info {
			0
		}
	}
};

