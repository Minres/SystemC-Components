/*******************************************************************************
 * Copyright (c) 2014 OPCoach.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *     Manumitting Technologies : Brian de Alwis for initial API and implementation
 *     OPCoach  : O.Prouvost fix bugs on hierarchy
 *******************************************************************************//* 
 * Handler to open up a configured preferences dialog.
 * Written by Brian de Alwis, Manumitting Technologies.
 * Placed in the public domain.
 * This code comes from : http://www.eclipse.org/forums/index.php/fa/4347/
 * and was referenced in the thread : http://www.eclipse.org/forums/index.php/m/750139/
 */
package com.opcoach.e4.preferences.handlers;

import javax.inject.Named;

import org.eclipse.e4.core.di.annotations.CanExecute;
import org.eclipse.e4.core.di.annotations.Execute;
import org.eclipse.e4.ui.services.IServiceConstants;
import org.eclipse.jface.preference.PreferenceDialog;
import org.eclipse.jface.preference.PreferenceManager;
import org.eclipse.jface.viewers.ViewerComparator;
import org.eclipse.swt.widgets.Shell;

import com.opcoach.e4.preferences.internal.E4PreferenceRegistry;


public class E4PreferencesHandler
{
	
	
	@CanExecute
	public boolean canExecute()
	{
		return true;	
	}
	
	@Execute
	public void execute(@Named(IServiceConstants.ACTIVE_SHELL) Shell shell,  E4PreferenceRegistry prefReg)
	{
		PreferenceManager pm = prefReg.getPreferenceManager();
		PreferenceDialog dialog = new PreferenceDialog(shell, pm);
		dialog.create();
		dialog.getTreeViewer().setComparator(new ViewerComparator());
		dialog.getTreeViewer().expandAll();
		dialog.open();
	}

	

	
}
