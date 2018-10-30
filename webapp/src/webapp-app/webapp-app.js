import { PolymerElement, html } from '@polymer/polymer/polymer-element.js';
import '@polymer/paper-button/paper-button.js';
import '@polymer/paper-styles/typography.js';
import '@polymer/paper-styles/typography.js';
import '@polymer/iron-ajax/iron-ajax.js';
import '@polymer/paper-card/paper-card.js';
import '@polymer/iron-pages/iron-pages.js';
import '@polymer/paper-input/paper-input.js';
import '@polymer/iron-flex-layout/iron-flex-layout-classes.js';
import '@polymer/paper-styles/color.js';
import '@polymer/paper-dialog/paper-dialog.js';
import '@polymer/paper-tabs/paper-tabs.js';
import '@polymer/paper-tabs/paper-tab.js';

/**
 * @customElement
 * @polymer
 */
class WebappApp extends PolymerElement {
  static get template() {
    return html`
    <style is="custom-style" include="iron-flex iron-flex-alignment">
      /* :host {
        transition: opacity 0.3s cubic-bezier(0.4, 0, 0.2, 1);
        --app-primary-color: var(--paper-indigo-500);
        --app-secondary-color: black;

      } */

      paper-tabs {
        background-color: var(--paper-blue-900);
        color: #fff;
      }

      paper-button{
        margin: 8px 8px 8px 8px
      }
      paper-button.blue {
        background: var(--paper-blue-700);
        color: white;
      }
      paper-button.green {
        background: var(--paper-green-700);
        color: white;
      }
      paper-button.red {
        background: var(--paper-red-700);
        color: white;
      }
      paper-button.orange {
        background: var(--paper-orange-700);
        color: white;
      }

      .content {
        @apply --layout-vertical;
        /* max-width: 500px; */
        padding: 16px;
        @apply --layout-center;     
      }

      .value {
        @apply --paper-font-subheading;
        font-weight: bold;
      }

      .label {
        @apply --paper-font-body2;
      }
      .subheading{
        @apply --paper-font-title;
        margin-bottom: 16px;
      }
    </style>
    <iron-ajax id="getDataAjax" auto="" url="[[_http_root]]all" handle-as="json" last-response="{{_data}}" debounce-duration="300" on-response="_onAllResponse">
    </iron-ajax>
    <iron-ajax id="getConfigAjax" auto="" url="[[_http_root]]config" handle-as="json" last-response="{{_config}}" debounce-duration="300">
    </iron-ajax>
    <iron-ajax id="saveConfigAjax" method="POST" url="[[_http_root]]save-config" content-type="application/json" debounce-duration="300" on-response="_onSaveConfigResponse" on-error="_onSaveConfigError">
    </iron-ajax>
    <iron-ajax id="rebootAjax" url="[[_http_root]]reboot" method="GET" debounce-duration="300" on-response="_onRebootResponse" on-error="_onRebootError">
    </iron-ajax>
    <iron-ajax id="doorActionAjax" url="[[_http_root]]action" handle-as="json" method="GET" params="[[_actionParams]]" debounce-duration="300" on-response="_onActionResponse"
      on-response="_onActionError">
    </iron-ajax>

    <paper-dialog id="_dialog" modal>
      <h2>[[_dialogTitle]]</h2>
      <p>[[_dialogMessage]]</p>
      <div class="buttons">
        <paper-button dialog-confirm autofocus>OK</paper-button>
      </div>
    </paper-dialog>

    <div class="content">
      <iron-pages id="_pages" selected="0">
        <paper-card heading="Garage Door">
          <div class="card-content">
              <div class="subheading">Current Information</div>
              <table fixed-column="">
              <tbody>
                <tr>
                  <td class="label">Timestamp</td>
                  <td class="value">[[_data.time_stamp]]</td>
                </tr>
                <tr>
                  <td class="label">Door Action</td>
                  <td class="value">[[_data.door_action]]</td>
                </tr>
                <tr>
                  <td class="label">Door Position</td>
                  <td class="value">[[_data.door_position]]</td>
                </tr>
                <tr>
                  <td class="label">Door Locked</td>
                  <td class="value">[[_true_false(_data.door_locked)]]</td>
                </tr>
                <tr>
                  <td class="label">In Home Area</td>
                  <td class="value">[[_true_false(_data.in_home_area)]]</td>
                </tr>
                <tr>
                  <td class="label">Temperature</td>
                  <td class="value">[[_data.temperature]]</td>
                </tr>
                <tr>
                  <td class="label">Up Time</td>
                  <td class="value">[[_data.up_time]]</td>
                </tr>
                <tr>
                  <td class="label">Boot Time</td>
                  <td class="value">[[_data.boot_time]]</td>
                </tr>
                <tr>
                  <td class="label">Free Heap</td>
                  <td class="value">[[_data.heap]]</td>
                </tr>
              </tbody>
            </table>
          </div>
          <div class="card-actions">
            <paper-button raised class="blue" disabled="[[_door_action_enabled(_data.door_position, _data.door_locked)]]" 
              on-tap="_toggle_door">[[_door_action_title(_data.door_position)]]</paper-button>
            <paper-button on-tap="_toggle_locked">[[_door_locked_title(_data.door_locked)]]</paper-button>
            <paper-button raised class="green" on-tap="_configure">Configure</paper-button>
          </div>
        </paper-card>

        <paper-card heading="Garage Door">
          <div class="card-content">
            <div class="subheading">Configuration</div>
            <paper-tabs selected="{{_selectedConfTab}}">
              <paper-tab>Network</paper-tab>
              <paper-tab>Adafruit IO</paper-tab>
              <paper-tab>InfluxDB</paper-tab>
            </paper-tabs>
            <iron-pages id="_ConfigPages" selected="{{_selectedConfTab}}">
              <div>
                  <paper-input always-float-label label="Hostname" value="{{_config.hostname}}"></paper-input>
                  <paper-input always-float-label label="WiFi AP Name" type="password" value="{{_config.wifi_ap}}"></paper-input>
                  <paper-input always-float-label label="WiFi Password" type="password" value="{{_config.wifi_password}}"></paper-input>
                  <paper-input always-float-label label="Update Interval (Seconds)" value="{{_config.update_interval}}"></paper-input>
              </div>
              <div>
                <paper-input always-float-label label="Username" value="{{_config.io_feed_username}}"></paper-input>
                <paper-input always-float-label label="Key" value="{{_config.io_feed_key}}"></paper-input>
                <paper-input always-float-label label="Feed Action" value="{{_config.io_feed_garagedooraction}}"></paper-input>
                <paper-input always-float-label label="Feed Position" value="{{_config.io_feed_garagedoorposition}}"></paper-input>
                <paper-input always-float-label label="Feed Geo Fence" value="{{_config.io_feed_inhomearea}}"></paper-input>
              </div>
              <div>
                  <paper-input always-float-label label="Host" value="{{_config.influx_host}}"></paper-input>
                  <paper-input always-float-label label="Port" value="{{_config.influx_port}}"></paper-input>
                  <paper-input always-float-label label="Username" value="{{_config.influx_username}}"></paper-input>
                  <paper-input always-float-label label="Password" type="password" value="{{_config.influx_password}}"></paper-input>
                  <paper-input always-float-label label="Database" value="{{_config.influx_database}}"></paper-input>
                  <paper-input always-float-label label="Measurement" value="{{_config.influx_measurement}}"></paper-input>
                  <paper-input always-float-label label="Door" value="{{_config.influx_door}}"></paper-input>
                  <paper-input always-float-label label="Temperature Location" value="{{_config.influx_temperature_location}}"></paper-input>
                  <paper-input always-float-label label="Temperature Measurement" value="{{_config.influx_temperature_measurement}}"></paper-input>
              </div>

            </iron-pages>            
          </div>
          <div class="card-actions">
            <div>
              <paper-button raised class="orange" on-tap="_go_back">Back</paper-button>
              <paper-button raised class="green" on-tap="_save">Save</paper-button>
            </div>
            <div>
              <!--paper-button raised class="red" on-tap="_reboot">Reboot</paper-button -->
            </div>
          </div>
        </paper-card>

      </iron-pages>
    </div>

`;
  }

  static get is() { return 'webapp-app'; }

  static get observers() {
    return [
      '_pageChanged(_page)'
    ];
  }
  
  constructor() {
    super();
    this._data = null;
    this._actionParams = null;
    this._config = null;
    this._dialogTitle = null;
    this._dialogMessage = null;
    this._http_root = window.MyAppGlobals.rootPath;
    this._selectedConfTab = 0;
    this._page = 0;
  }

  _pageChanged(page){
    if (page == 1){ 
      this.$.getConfigAjax.generateRequest();
    }
    if(page == 0){
      this.$.getDataAjax.generateRequest();
    }
  }

  _onReadNowResponse(event) {
    this.$.getDataAjax.generateRequest();
  }

  _onReadNowError(event, error){
    this._showMessage("Read Now Error",error.error);
  }

  _configure() {
    this.$._pages.selected = 1;
  }

  _go_back() {
    this.$._pages.selected = 0;
  }

  _showMessage(title, message){
    this._dialogTitle = title;
    this._dialogMessage = message;
    this.$._dialog.open();
  }

  _onAllResponse(r) {
    console.log("response", r);
    this._intervalID = window.setTimeout(() => {
      this.$.getDataAjax.generateRequest();
    }, 5000);
  }


  _save(){
    this.$.saveConfigAjax.body = this._config;
    console.log("Sending " + this.$.saveConfigAjax.body);
    this.$.saveConfigAjax.generateRequest();
  }

  _onSaveConfigResponse(event){
    console.log("Save Response", event);
    this._showMessage("Save Config",event.detail.statusText);
  }

  _onSaveConfigError(event, error){
    this._showMessage("Save Error",error.error);
  }

  _reboot(){
    this.$.rebootAjax.generateRequest();
  }

  _onRebootResponse(event) {
    console.log("Reboot Response", event);
    this._showMessage("Reboot",event.detail.statusText);

  }

  _onRebootError(event, error){
    this._showMessage("Reboot Error",error.error);
  }


  _onActionResponse(event) {
    console.log("Action Response", event);
    this._showMessage("Action",event.detail.statusText);

  }

  _onActionError(event, error){
    this._showMessage("Action Error",error.error);
  }

  _true_false(home) {
    if (home != true && home != false) {
      return "Unknown"
    }
    return home ? "Yes" : "No";
  }

  _door_action_enabled(position, locked) {
    return (position !== "OPEN" && position !== "CLOSED") ||locked;
  }

  _door_action_title(position) {
    if (position === "OPEN") {
      return "Close Door"
    } else if (position === "CLOSED") {
      return "Open Door";
    } if (position === "" || position == null) {
      return "Unknown";
    }
    return "Moving";
  }

  _door_locked_title(locked){
    if(locked === true){
      return 'Unlock';
    } else if (locked == false){
      return 'Lock';
    }
    return 'Noo!!!!';

  }

  _toggle_door() {
    this._actionParams = {
      "door_action": this._data.door_position === "OPEN" ? "CLOSE" : "OPEN"
    };
    this.$.doorActionAjax.generateRequest();
  }

  _toggle_locked() {
    this._actionParams = {
      "lock_action": this._data.door_locked ? "UNLOCK" : "LOCK"
    };
    this.$.doorActionAjax.generateRequest();
    
  }


}


window.customElements.define(WebappApp.is, WebappApp);
