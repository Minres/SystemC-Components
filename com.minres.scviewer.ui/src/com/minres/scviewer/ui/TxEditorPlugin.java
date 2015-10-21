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
package com.minres.scviewer.ui;

import java.io.File;
import java.util.MissingResourceException;
import java.util.ResourceBundle;

import org.eclipse.jface.resource.ImageDescriptor;
import org.eclipse.swt.SWT;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.Image;
import org.eclipse.ui.plugin.AbstractUIPlugin;
import org.eclipse.wb.swt.SWTResourceManager;
import org.osgi.framework.BundleContext;

/**
 * The activator class controls the plug-in life cycle
 */
public class TxEditorPlugin extends AbstractUIPlugin {

	public static final int lineColor=0;
	public static final int txBgColor=1;
	public static final int highliteLineColor=2;
	public static final int txHighliteBgColor=3;
	public static final int trackBgLightColor=4;
	public static final int trackBgDarkColor=5;
	public static final int headerBgColor=6;
	public static final int headerFgColor=7;

	// The plug-in ID
	public static final String PLUGIN_ID = "com.minres.scviewer.ui"; //$NON-NLS-1$

	// The shared instance
	private static TxEditorPlugin plugin;
		
	private ResourceBundle resourceBundle;
	
	/**
	 * The constructor
	 */
//	public TxEditorPlugin() {
//		openedTxEditorPart=null;
//	}

	/*
	 * (non-Javadoc)
	 * @see org.eclipse.ui.plugin.AbstractUIPlugin#start(org.osgi.framework.BundleContext)
	 */
	public void start(BundleContext context) throws Exception {
		super.start(context);
		plugin = this;
		try {
			resourceBundle = ResourceBundle.getBundle(plugin.getClass().getName());
		} catch (MissingResourceException x) {
			resourceBundle = null;
		}	
	}

	/*
	 * (non-Javadoc)
	 * @see org.eclipse.ui.plugin.AbstractUIPlugin#stop(org.osgi.framework.BundleContext)
	 */
	public void stop(BundleContext context) throws Exception {
		plugin = null;
		SWTResourceManager.dispose();
		super.stop(context);
	}

	/**
	 * Returns the shared instance
	 *
	 * @return the shared instance
	 */
	public static TxEditorPlugin getDefault() {
		return plugin;
	}

	/**
	 * Returns an image descriptor for the image file at the given
	 * plug-in relative path
	 *
	 * @param path the path
	 * @return the image descriptor
	 */
	public static ImageDescriptor getImageDescriptor(String path) {
		String fullpath = File.separator+"res"+File.separator+"images"+File.separator+path;
		return imageDescriptorFromPlugin(PLUGIN_ID, fullpath);
	}
	
	public static Image createImage(String name) {
		ImageDescriptor id=getImageDescriptor(name+".png");
		return id.createImage();
	}

	public Color getColor(int idx){
		switch (idx) {
		case lineColor:
			return SWTResourceManager.getColor(SWT.COLOR_RED);
		case txBgColor:
			return SWTResourceManager.getColor(SWT.COLOR_GREEN);
		case highliteLineColor:
			return SWTResourceManager.getColor(SWT.COLOR_CYAN);
		case txHighliteBgColor:
			return SWTResourceManager.getColor(SWT.COLOR_DARK_GREEN);
		case trackBgLightColor:
			return SWTResourceManager.getColor(220, 220, 220);
		case trackBgDarkColor:
//			return SWTResourceManager.getColor(200, 200, 200);
			return SWTResourceManager.getColor(SWT.COLOR_BLACK);
		case headerBgColor:
			return SWTResourceManager.getColor(255, 255, 255);
		case headerFgColor:
			return SWTResourceManager.getColor(55, 55, 55);
		default:
			break;
		}
		return SWTResourceManager.getColor(SWT.COLOR_BLACK);
	}

	public ResourceBundle getResourceBundle() {
		return resourceBundle;
	}

}
