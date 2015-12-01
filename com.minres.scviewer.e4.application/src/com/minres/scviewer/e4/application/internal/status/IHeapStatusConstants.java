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
package com.minres.scviewer.e4.application.internal.status;
/**
 * Preference constants for the heap status.
 */
public interface IHeapStatusConstants {

	/**
	 * Preference key for the update interval (value in milliseconds).
	 */
    String PREF_UPDATE_INTERVAL = "HeapStatus.updateInterval"; //$NON-NLS-1$

    /**
     * Preference key for whether to show max heap, if available (value is boolean).
     */
    String PREF_SHOW_MAX = "HeapStatus.showMax";   //$NON-NLS-1$

}
