// Fill out your copyright notice in the Description page of Project Settings.


#include "Materials/AGX_ContactMaterialMechanicsApproach.h"


FAGX_ContactMaterialMechanicsApproach::FAGX_ContactMaterialMechanicsApproach()
	:
bUseContactAreaApproach(false),
MinElasticRestLength(2.0 * 0.0005),
MaxElasticRestLength(2.0 * 0.05)
{
	// See agx\src\agx\Material.cpp for default values
}
