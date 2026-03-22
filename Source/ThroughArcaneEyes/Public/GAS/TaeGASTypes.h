// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#pragma once

// Generates boilerplate attribute accessors for a UAttributeSet property.
// Place once per attribute in the attribute set class body.
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)
