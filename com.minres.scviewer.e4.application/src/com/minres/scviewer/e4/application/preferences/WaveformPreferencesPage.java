/*******************************************************************************
 * Copyright (c) 2015 MINRES Technologies GmbH and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *     MINRES Technologies GmbH - initial API and implementation
 *******************************************************************************/
package com.minres.scviewer.e4.application.preferences;

import org.eclipse.jface.preference.ColorFieldEditor;
import org.eclipse.jface.preference.FieldEditorPreferencePage;

import com.minres.scviewer.database.ui.WaveformColors;

/**
 *  The WaveformView preference page to show the colors to use.
 */
public class WaveformPreferencesPage extends FieldEditorPreferencePage {

	/**
	 * Instantiates a new waveform preferences page.
	 */
	public WaveformPreferencesPage() {
		super(GRID);
	}

	/* (non-Javadoc)
	 * @see org.eclipse.jface.preference.FieldEditorPreferencePage#createFieldEditors()
	 */
	@Override
	protected void createFieldEditors() {

		for (WaveformColors c : WaveformColors.values()) {
			addField(new ColorFieldEditor(c.name() + "_COLOR", "Color for " + c.name().toLowerCase(),
					getFieldEditorParent()));
		}
	}

}
