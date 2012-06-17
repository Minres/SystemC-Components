package com.itjw.txviewer.graph;

import java.io.File;
import java.util.MissingResourceException;
import java.util.ResourceBundle;

import org.eclipse.draw2d.ColorConstants;
import org.eclipse.jface.resource.ImageDescriptor;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.Image;
import org.eclipse.ui.plugin.AbstractUIPlugin;
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
	public static final String PLUGIN_ID = "com.itjw.txviewer.graph"; //$NON-NLS-1$

	// The shared instance
	private static TxEditorPlugin plugin;
		
	private ResourceBundle resourceBundle;
	
	private TxEditorPart openedTxEditorPart;

	/**
	 * The constructor
	 */
	public TxEditorPlugin() {
		openedTxEditorPart=null;
	}

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
			return ColorConstants.red;
		case txBgColor:
			return ColorConstants.green;
		case highliteLineColor:
			return ColorConstants.cyan;
		case txHighliteBgColor:
			return ColorConstants.darkGreen;
		case trackBgLightColor:
			return new Color(null, 220, 220, 220);
		case trackBgDarkColor:
			return new Color(null, 200, 200, 200);
		case headerBgColor:
			return new Color(null, 255, 255, 255);
		case headerFgColor:
			return new Color(null, 55, 55, 55);
		default:
			break;
		}
		return ColorConstants.black;
	}

	public ResourceBundle getResourceBundle() {
		return resourceBundle;
	}

	public void editorOpened(TxEditorPart txEditorPart) {
		openedTxEditorPart=txEditorPart;
	}
	
	public TxEditorPart getOpenEditorPart(){
		return openedTxEditorPart;
	}
}
