// Fill out your copyright notice in the Description page of Project Settings.


#include "Materials/AGX_MaterialBulkProperties.h"


FAGX_MaterialBulkProperties::FAGX_MaterialBulkProperties()
	:
Density(1000.0),
YoungsModulus(2.0 / 5.0E-9),
Viscosity(0.5),
Damping(4.5 / 60.0),
MinElasticRestLength(0.0005),
MaxElasticRestLength(0.05)
{

}