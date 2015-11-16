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
package com.minres.scviewer.e4.application.parts;

import java.awt.Desktop;
import java.io.IOException;
import java.net.URISyntaxException;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.eclipse.swt.SWT;
import org.eclipse.swt.custom.StyleRange;
import org.eclipse.swt.custom.StyledText;
import org.eclipse.swt.events.PaintEvent;
import org.eclipse.swt.events.PaintListener;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Canvas;
import org.eclipse.swt.widgets.Dialog;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Event;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Link;
import org.eclipse.swt.widgets.Listener;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.wb.swt.ResourceManager;
import org.eclipse.wb.swt.SWTResourceManager;

public class AboutDialog extends Dialog {

	protected int result;
	protected Shell shell;
	private Color white;
	protected StyledText styledText;
	/*
	Eclipse IDE for Java Developers

Version: Mars.1 Release (4.5.1)
Build id: 20150924-1200
(c) Copyright Eclipse contributors and others 2000, 2015.  All rights reserved. Eclipse and the Eclipse logo are trademarks of the Eclipse Foundation, Inc., https://www.eclipse.org/. The Eclipse logo cannot be altered without Eclipse's permission. Eclipse logos are provided for use under the Eclipse logo and trademark guidelines, https://www.eclipse.org/logotm/. Oracle and Java are trademarks or registered trademarks of Oracle and/or its affiliates. Other names may be trademarks of their respective owners.
	*/
	private String productTitle=
			"\nSCViewer - a SystemC waveform viewer\n\nVersion: 1.0\n";
	private String copyrightText="\nCopyright (c) 2015 MINRES Technologies GmbH and others.\n"+
					"\n"+
					"All rights reserved. MINRES and the MINRES logo are trademarks of MINRES Technologies GmbH, http://www.minres.com/ . "+
					"This program and the accompanying materials are made available under the terms of the Eclipse Public License v1.0 "+
					"which accompanies this distribution, and is available at http://www.eclipse.org/legal/epl-v10.html\n"+
					"\n\nSources code is hosted at GitHub: https://github.com/eyck/txviewer\n";

	/**
	 * Create the dialog.
	 * @param parent
	 * @param style
	 */
	public AboutDialog(Shell parent, int style) {
		super(parent, style);
		setText("SWT Dialog");
		white=SWTResourceManager.getColor(SWT.COLOR_WHITE);
	}

	/**
	 * Open the dialog.
	 * @return the result
	 */
	public int open() {
		createContents();
		shell.open();
		shell.layout();
		Display display = getParent().getDisplay();
		while (!shell.isDisposed()) {
			if (!display.readAndDispatch()) {
				display.sleep();
			}
		}
		return result;
	}

	/**
	 * Create contents of the dialog.
	 */
	private void createContents() {
		shell = new Shell(getParent(), getStyle());
		shell.setSize(600, 300);
		shell.setText(getText());
		shell.setLayout(new GridLayout(2, false));
		final Image scviewerLogo=ResourceManager.getPluginImage("com.minres.scviewer.e4.application", "icons/SCViewer_logo.png");
		final Image minresLogo=ResourceManager.getPluginImage("com.minres.scviewer.e4.application", "icons/Minres_logo.png");

		Canvas canvas = new Canvas(shell,SWT.NO_REDRAW_RESIZE);
		GridData gd_canvas = new GridData(SWT.LEFT, SWT.CENTER, false, false, 1, 1);
		gd_canvas.widthHint = 200;
		gd_canvas.heightHint =250;
		canvas.setLayoutData(gd_canvas);
		canvas.addPaintListener(new PaintListener() {
			public void paintControl(PaintEvent e) {
				e.gc.setBackground(white);
				e.gc.fillRectangle(e.x, e.y, e.width, e.height);
				e.gc.drawImage(scviewerLogo,4,0);
				e.gc.drawImage(minresLogo,0,200);
			}
		});

		styledText = new StyledText(shell, SWT.BORDER);
		styledText.setEditable(false);
		GridData gd_styledText = new GridData(SWT.FILL, SWT.FILL, true, true, 1, 1);
		styledText.setLayoutData(gd_styledText);
		styledText.setText(productTitle+copyrightText);
		styledText.setBackground(white);
		styledText.setWordWrap(true);
		styledText.setLeftMargin(5);
		StyleRange styleRange = new StyleRange();
		styleRange.start = 0;
		styleRange.length = productTitle.length();
		styleRange.fontStyle = SWT.BOLD;
		styledText.setStyleRange(styleRange);
		///^(https?:\/\/)?([\da-z\.-]+)\.([a-z\.]{2,6})([\/\w \.-]*)*\/?$/
	    Pattern pattern = Pattern.compile("https?:\\/\\/([\\da-z\\.-]+)\\.([a-z\\.]{2,6})([\\/\\w\\.-]*)*\\/?");
	    // in case you would like to ignore case sensitivity,
	    // you could use this statement:
	    // Pattern pattern = Pattern.compile("\\s+", Pattern.CASE_INSENSITIVE);
	    Matcher matcher = pattern.matcher(productTitle+copyrightText);
	    // check all occurance
	    while (matcher.find()) {
			styleRange = new StyleRange();
			styleRange.underline=true;
			styleRange.underlineStyle = SWT.UNDERLINE_LINK;
			styleRange.data = matcher.group();
			styleRange.start = matcher.start();
			styleRange.length = matcher.end()-matcher.start();
			styledText.setStyleRange(styleRange);
	    }
		styledText.addListener(SWT.MouseDown, new Listener() {
			@Override
			public void handleEvent(Event event) {
				// It is up to the application to determine when and how a link should be activated.
				// links are activated on mouse down when the control key is held down 
//				if ((event.stateMask & SWT.MOD1) != 0) {
					try {
						int offset = styledText.getOffsetAtLocation(new Point (event.x, event.y));
						StyleRange style = styledText.getStyleRangeAtOffset(offset);
						if (style != null && style.underline && style.underlineStyle == SWT.UNDERLINE_LINK) {
							Desktop.getDesktop().browse(new java.net.URI(style.data.toString()));
						}
					} catch (IOException | URISyntaxException | IllegalArgumentException e) {}
//				}
			}
		});
		
		styleRange.start = 0;
		new Label(shell, SWT.NONE);

		Button okButton = new Button(shell, SWT.NONE);
		okButton.setLayoutData(new GridData(SWT.RIGHT, SWT.CENTER, false, false, 1, 1));
		okButton.setBounds(0, 0, 94, 28);
		okButton.setText("Close");
		okButton.setFocus();
		okButton.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent e) {
				if(!shell.isDisposed()) shell.dispose();
			}
		});
	}

	public static boolean open(Shell parent, int style) {
		AboutDialog dialog = new AboutDialog(parent, style | SWT.SHEET);
		return dialog.open() == 0;
	}
}
