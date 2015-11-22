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

import org.eclipse.core.runtime.preferences.AbstractPreferenceInitializer;
import org.eclipse.core.runtime.preferences.InstanceScope;
import org.eclipse.jface.preference.IPreferenceStore;
import org.eclipse.jface.resource.StringConverter;
import org.eclipse.swt.SWT;
import org.eclipse.swt.graphics.Color;
import org.eclipse.wb.swt.SWTResourceManager;

import com.minres.scviewer.database.ui.WaveformColors;
import com.opcoach.e4.preferences.ScopedPreferenceStore;

/**
 * The Class DefaultValuesInitializer.
 */
public class DefaultValuesInitializer extends AbstractPreferenceInitializer {

    /** The default colors. */
    public final Color[] colors = new Color[WaveformColors.values().length];

	/**
	 * Instantiates a new default values initializer.
	 */
	public DefaultValuesInitializer() {
        colors[WaveformColors.LINE.ordinal()] = SWTResourceManager.getColor(SWT.COLOR_RED);
        colors[WaveformColors.LINE_HIGHLITE.ordinal()] = SWTResourceManager.getColor(SWT.COLOR_CYAN);
        colors[WaveformColors.TRACK_BG_EVEN.ordinal()] = SWTResourceManager.getColor(SWT.COLOR_BLACK);
        colors[WaveformColors.TRACK_BG_ODD.ordinal()] = SWTResourceManager.getColor(40, 40, 40);
        colors[WaveformColors.TRACK_BG_HIGHLITE.ordinal()] = SWTResourceManager.getColor(40, 40, 80);
        colors[WaveformColors.TX_BG.ordinal()] = SWTResourceManager.getColor(SWT.COLOR_GREEN);
        colors[WaveformColors.TX_BG_HIGHLITE.ordinal()] = SWTResourceManager.getColor(SWT.COLOR_DARK_GREEN);
        colors[WaveformColors.TX_BORDER.ordinal()] = SWTResourceManager.getColor(SWT.COLOR_RED);
        colors[WaveformColors.SIGNAL0.ordinal()] = SWTResourceManager.getColor(SWT.COLOR_DARK_GREEN);
        colors[WaveformColors.SIGNAL1.ordinal()] = SWTResourceManager.getColor(SWT.COLOR_DARK_GREEN);
        colors[WaveformColors.SIGNALZ.ordinal()] = SWTResourceManager.getColor(SWT.COLOR_GRAY);
        colors[WaveformColors.SIGNALX.ordinal()] = SWTResourceManager.getColor(255,  128,  182);
        colors[WaveformColors.SIGNAL_TEXT.ordinal()] = SWTResourceManager.getColor(SWT.COLOR_WHITE);
        colors[WaveformColors.CURSOR.ordinal()] = SWTResourceManager.getColor(SWT.COLOR_RED);
        colors[WaveformColors.CURSOR_DRAG.ordinal()] = SWTResourceManager.getColor(SWT.COLOR_GRAY);
        colors[WaveformColors.CURSOR_TEXT.ordinal()] = SWTResourceManager.getColor(SWT.COLOR_WHITE);
        colors[WaveformColors.MARKER.ordinal()] = SWTResourceManager.getColor(SWT.COLOR_DARK_GRAY);
        colors[WaveformColors.MARKER_TEXT.ordinal()] = SWTResourceManager.getColor(SWT.COLOR_WHITE);
        colors[WaveformColors.REL_ARROW.ordinal()] = SWTResourceManager.getColor(SWT.COLOR_MAGENTA);
        colors[WaveformColors.REL_ARROW_HIGHLITE.ordinal()] = SWTResourceManager.getColor(255, 128, 255);
	}

	/* (non-Javadoc)
	 * @see org.eclipse.core.runtime.preferences.AbstractPreferenceInitializer#initializeDefaultPreferences()
	 */
	@Override
	public void initializeDefaultPreferences() {
		IPreferenceStore store = new ScopedPreferenceStore(InstanceScope.INSTANCE, PreferenceConstants.PREFERENCES_SCOPE);

		store.setDefault(PreferenceConstants.DATABASE_RELOAD, true);
        for (WaveformColors c : WaveformColors.values()) {
        	 store.setDefault(c.name()+"_COLOR", StringConverter.asString(colors[c.ordinal()].getRGB()));
        }
	}

}
