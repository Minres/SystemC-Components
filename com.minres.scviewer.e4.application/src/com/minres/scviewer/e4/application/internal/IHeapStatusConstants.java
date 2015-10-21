package com.minres.scviewer.e4.application.internal;
/**
 * Preference constants for the heap status.
 *
 * @since 3.1
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
