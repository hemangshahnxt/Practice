#pragma once

namespace LabUtils
{
	// Loads the lab label print settings and displays the print dialog
	void ShowLabLabelPrintSettings();

	// Stores the lab label printer settings based on the System Property Specificity preference.
	// - Server: Stored in the registry on the client or terminal server
	// - Client: Stored in ConfigRT based on the local machine name
	BOOL SaveLabLabelPrintSettings(HGLOBAL &hDevNames, HGLOBAL &hDevMode);

	// Retrieves the label printer settings based on the System Property Specificity preference.
	// - Server: Stored in the registry on the client or terminal server
	// - Client: Stored in ConfigRT based on the local machine name
	void GetLabLabelPrintSettings(OUT HGLOBAL &hDevNames, OUT HGLOBAL &hDevMode);

	// Returns a _variant_t based on the provided HGLOBAL
	_variant_t DevObjectToVariant(const HGLOBAL &hDevObject);
}