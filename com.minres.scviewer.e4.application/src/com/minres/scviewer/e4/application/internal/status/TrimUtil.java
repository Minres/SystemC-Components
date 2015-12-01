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
import org.eclipse.jface.dialogs.Dialog;
import org.eclipse.jface.resource.JFaceResources;
import org.eclipse.swt.SWT;
import org.eclipse.swt.graphics.GC;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.swt.widgets.ToolBar;
import org.eclipse.swt.widgets.ToolItem;

/**
 * Simple class to provide some common internal Trim support.
 *
 */
public class TrimUtil {

    /**
     * Default height for workbench trim.
     */
    public static final int TRIM_DEFAULT_HEIGHT;
    static {
    	Shell s = new Shell(Display.getCurrent(), SWT.NONE);
    	s.setLayout(new GridLayout());
    	ToolBar t = new ToolBar(s, SWT.NONE);
    	t.setLayoutData(new GridData(SWT.FILL, SWT.FILL, true, true));
    	ToolItem ti = new ToolItem(t, SWT.PUSH);
    	ti.setImage(JFaceResources.getImageRegistry().get(Dialog.DLG_IMG_MESSAGE_INFO));
    	s.layout();
    	int toolItemHeight = t.computeSize(SWT.DEFAULT, SWT.DEFAULT).y;
    	GC gc = new GC(s);
    	Point fontSize = gc.textExtent("Wg"); //$NON-NLS-1$
    	gc.dispose();
    	TRIM_DEFAULT_HEIGHT = Math.max(toolItemHeight, fontSize.y);
    	s.dispose();

    }
}
