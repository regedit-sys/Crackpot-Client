#include "stdafx.h"
#include "common.h"

const size_t register_emailMaxLen = 32;
const size_t register_nameMaxLen = 15;
void REGISTER_NAME(PWCHAR defaultName, PWCHAR defaultEmail);
XOVERLAPPED expiredMsg_confirm;
MESSAGEBOX_RESULT expiredMsg_result_confirm;

XOVERLAPPED overLapped_errorTkn;
XOVERLAPPED overLapped_errorRegister;
XOVERLAPPED overLapped_confirm;
XOVERLAPPED overLapped_check;
XOVERLAPPED overLapped_email;
XOVERLAPPED overLapped_name;

MESSAGEBOX_RESULT resultBox_errorTkn;
MESSAGEBOX_RESULT resultBox_errorRegister;
MESSAGEBOX_RESULT resultBox_tknConfirm;

void TOKEN_ERROR(wchar_t *key, wchar_t *msg) {
	LPCWSTR buttons[2] = { L"Try Again", L"Close" };
	XShowMessageBoxUI(0, L"Error", msg, 2, buttons, 0, XMB_ALERTICON, &resultBox_errorTkn, &overLapped_errorTkn);
	while (!XHasOverlappedIoCompleted(&overLapped_errorTkn)) Native::Sleep(500);
	if (resultBox_errorTkn.dwButtonPressed == 0) {
		Native::Sleep(550);
		TOKEN_CHECK(key);
	}
}

void REGISTER_ERROR(wchar_t *email, wchar_t *name, wchar_t *msg, short id = 0) {
	LPCWSTR buttons[2] = { L"Try Again", L"Close" };
	XShowMessageBoxUI(0, L"Error", msg, 2, buttons, 0, XMB_ALERTICON, &resultBox_errorRegister, &overLapped_errorRegister);
	while (!XHasOverlappedIoCompleted(&overLapped_errorRegister)) Native::Sleep(500);
	if (resultBox_errorRegister.dwButtonPressed == 0) {
		Native::Sleep(550);
		if(id==0) REGISTER_EMAIL(email);
		else REGISTER_NAME(name, email);
	}
}

void TOKEN_CONFIRM(char *token, int days){
	wchar_t msg[150];
	int status = -1;
	if (days > 998) {
		swprintf(msg, L"Click 'Redeem' below to confirm your purchase and add time to your account or press 'Close' to cancel.\n\nLifetime will be redeemed");
	}else swprintf(msg, L"Click 'Redeem' below to confirm your purchase and add time to your account or press 'Close' to cancel.\n\n%d day(s) will be redeemed", days);
	LPCWSTR buttons[2] = {L"Redeem", L"Close"};

	XShowMessageBoxUI(0, L"Confirm Purchase", msg, 2, buttons, 0, XMB_ALERTICON, &resultBox_tknConfirm, &overLapped_confirm);
	while(!XHasOverlappedIoCompleted(&overLapped_confirm)) Native::Sleep(500);
	if(resultBox_tknConfirm.dwButtonPressed == 0){
	if((status = NET::TOKEN(token, true)) == -2) return;
		if(status == -1) FNotify(L"This token does not exist anymore").error();
		else if(status == 0) FNotify(L"This token has already been redeemed!").error();
		else {
			if (Teapot::AuthFlag != AUTHFLAG_ACTIVE) {
				FNotify(L"Token has been redeemed to your account, rebooting to finalize...").show(true);
			}else FNotify(L"Token has been redeemed to your account!").show();
		}
	}
}

void TOKEN_CHECK(PWCHAR defaultKey){
	const size_t tokenMaxLen = 21;
	wchar_t tokenInput[tokenMaxLen];
	char token[tokenMaxLen];
	int status = -1;

	XShowKeyboardUI(0, VKBD_LATIN_FULL, defaultKey, L"Redeem Token", L"Enter a valid token to redeem time to your account\nEx: AAAAA-BBBBB-CCCCC", tokenInput, tokenMaxLen, &overLapped_check);
	while (!XHasOverlappedIoCompleted(&overLapped_check)) Native::Sleep(500);
	debug("[USR] Token Check: %ws", tokenInput);
	if(wcslen(tokenInput)!=0){
		wcstombs(token, tokenInput, tokenMaxLen);
		if (strlen(token) <= 2) return;
		if(strlen(token)!=20){
			Native::Sleep(550);
			TOKEN_ERROR(tokenInput, L"The token you have entered is too short!");
			return;
		}else if(token[6]!= '-'||token[13]!='-'){
			Native::Sleep(550);
			TOKEN_ERROR(tokenInput, L"The token you have entered does not appear to be in correct format");
			return;
		}else if ((status = NET::TOKEN(token)) == -2) {
			Native::Sleep(550);
			TOKEN_ERROR(tokenInput, L"Network Error - Please Try again!");
			return;
		}else if(status == -1){
			Native::Sleep(550);
			TOKEN_ERROR(tokenInput, L"The token you have entered is not valid");
			return;
		}else if (status == -3) {
			Native::Sleep(550);
			TOKEN_ERROR(tokenInput, L"You may not redeem CORE tokens on the premium service.\n\nPlease switch to CORE if you would like to use this token.\n\nhttps://Crackpot.live/");
			return;
		}else if(status == 0){
			Native::Sleep(550);
			FNotify(L"This token has already been redeemed!").error();
			return;
		}else {
			Native::Sleep(550);
			TOKEN_CONFIRM(token, status);
		}
	}
}

void REGISTER_EMAIL(PWCHAR defaultEmail) {
	wchar_t emailInput[register_emailMaxLen] = { L"" };
	char email[register_emailMaxLen] = { "" };
	int status = -1;

	PWCHAR detailsTxt = L"Enter your email address then press continue\nYour email will be used to recover any account info\nYour email must be 2-32 characters in length";
	XShowKeyboardUI(0, VKBD_LATIN_FULL, defaultEmail, L"Register - Crackpot account", detailsTxt, emailInput, register_emailMaxLen, &overLapped_email);
	while (!XHasOverlappedIoCompleted(&overLapped_email)) Native::Sleep(500);
	if (wcslen(emailInput) != 0) {
		wcstombs(email, emailInput, register_emailMaxLen);
		if (strlen(email) < 2) {
			Native::Sleep(550);
			REGISTER_ERROR(emailInput, L"", L"Your email must be between 2-32 characters!");
			return;
		}

		status = NET::REGISTER(email);
		if (status == -4) {
			REGISTER_ERROR(emailInput, L"", L"Something went wrong! please download the latest files from our discord.");
		}else if (status == -3) {
			REGISTER_ERROR(emailInput, L"", L"Registration is currently disabled.\nPlease contact an administrator!");
		}else if (status == -2) {
			Native::Sleep(550);
			REGISTER_ERROR(emailInput, L"", L"Your email address appears to be invalid.\nPlease re-check it (email@example.com)");
		}else if (status == -1) {
			Native::Sleep(550);
			REGISTER_ERROR(emailInput, L"", L"Previous registration Canceled.\nPlease shutdown and wait 5 minutes and try again.");
		}else if (status == 0) {
			Native::Sleep(550);
			REGISTER_ERROR(emailInput, L"", L"The email you entered is associated with another CPU key!");
		}else if(status == 1) {
			Native::Sleep(550);
			REGISTER_NAME(L"No Name", emailInput);
		}
	}else {
		Native::Sleep(550);
		REGISTER_ERROR(emailInput, L"", L"You cannot leave email blank!");
	}
}

void REGISTER_NAME(PWCHAR defaultName, PWCHAR emailInput) {
	wchar_t nameInput[register_nameMaxLen] = { L"" };
	char name[register_nameMaxLen] = { "" };
	int status = -1;

	XShowKeyboardUI(0, VKBD_LATIN_FULL, defaultName, L"Register", L"Enter a username between 2-15 characters\nYour username will be shown when you boot your console", nameInput, register_nameMaxLen, &overLapped_name);
	while (!XHasOverlappedIoCompleted(&overLapped_name)) Native::Sleep(500);
	if (wcslen(nameInput) != 0){
		wcstombs(name, nameInput, register_nameMaxLen);
		if (strlen(name) < 2){
			Native::Sleep(550);
			REGISTER_ERROR(emailInput, nameInput, L"Your username must be between 2-15 characters!", 1);
			return;
		}

		char email[register_emailMaxLen];
		wcstombs(email, emailInput, register_emailMaxLen);
		status = NET::REGISTER(email, name, 1);
		if (status == -4) {
			REGISTER_ERROR(emailInput, L"", L"Something went wrong, please download the latest files from our discord.");
		}else if (status == -3) {
			REGISTER_ERROR(emailInput, nameInput, L"Registration is currently disabled.\nPlease contact an administrator!", 1);
		}
		else if (status == -1){
			Native::Sleep(550);
			REGISTER_ERROR(emailInput, nameInput, L"An unknown error has occurred!\nPlease try again", 1);
		}else if (status == 1){
			Native::Sleep(550);

			Mem::Null(registration::registrationKey, 0x10);
			free(registration::registrationKey);
			Teapot::AuthFlag = AUTHFLAG_REBOOTPENDING;
			FNotify(L"Welcome to Crackpot!").show();
			Native::Sleep(3500);
			FNotify(L"You have been registered!\nRebooting to finalize!").show(true);
		}
	}else{
		Native::Sleep(550);
		REGISTER_ERROR(emailInput, nameInput, L"You cannot leave your name blank!", 1);
	}
}

void UI_ExpiredMsg() {
	DLaunch.SetLiveBlock(true);
	Teapot::AuthFlag = AUTHFLAG_EXPIRED;

	LPCWSTR buttons[1] = { L"Okay" };

	wstring msg(L"Your time has expired.\n\n");
	msg.append(L"You can get more time at: \nhttps://Crackpot.live\n");
	msg.append(L"-Choose your desired package.\n");
	msg.append(L"-Enter the email you wish to have your token delivered to.\n");
	msg.append(L"-CashApp & All Credit Cards accepted!\n");
	msg.append(L"-Make sure to check your spam folder!\n\n");

	msg.append(L"If you have any questions or need support please join our discord at: https://Crackpot.live/discord\n\n");

	msg.append(L"Thank you!");

	XShowMessageBoxUI(0, L"Crackpot - Time Expired!", msg.c_str(), 1, buttons, 0, XMB_WARNINGICON, &expiredMsg_result_confirm, &expiredMsg_confirm);
	while (!XHasOverlappedIoCompleted(&expiredMsg_confirm)) Native::Sleep(500);

	Native::Sleep(1500);
	TOKEN_CHECK(L"");
}