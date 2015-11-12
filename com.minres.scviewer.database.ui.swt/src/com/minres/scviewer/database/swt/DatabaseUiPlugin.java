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
package com.minres.scviewer.database.swt;

import org.eclipse.core.runtime.IStatus;
import org.eclipse.core.runtime.Plugin;
import org.eclipse.core.runtime.Status;
import org.osgi.framework.BundleContext;

public class DatabaseUiPlugin extends Plugin {
	
  public void start(BundleContext context) throws Exception {
    getLog().log(new Status(IStatus.OK, "org.eclipse.e4.core", "Starting org.eclipse.e4.core bundle..."));
  }
 
  public void stop(BundleContext context) throws Exception {
    getLog().log(new Status(IStatus.OK, "org.eclipse.e4.core", "Stopping org.eclipse.e4.core bundle..."));
  }
}