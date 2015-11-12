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

import org.eclipse.jface.preference.BooleanFieldEditor;
import org.eclipse.jface.preference.FieldEditorPreferencePage;

/** A sample preference page to show how it works */
public class SCViewerPreferencesPage extends FieldEditorPreferencePage {

	public SCViewerPreferencesPage() {
		super(GRID);
	}

	@Override
	protected void createFieldEditors() {

		addField(new BooleanFieldEditor(PreferenceConstants.DATABASE_RELOAD, "Check for changed database",
				getFieldEditorParent()));

	}

}
